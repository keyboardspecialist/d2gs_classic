# Classic 1.09 Adapter Notes

## Runtime identity

The staged classic DLL set is dated August 16, 2001. The repaired client uses
`Game.exe` version `1.0.9.22` (SHA-256
`8B120803FF6D7D4A7413445665BDBAC0A088157B69F6D52B178FC9E12A2049C9`).
PvPGN identifies it as `D2DV_109B`. The bundled engine reports:

```text
D2GSLib v1.09b13, built January 31, 2002
Core Game Version: Diablo2 LOD v1.09d
```

Relevant SHA-256 hashes:

```text
D2Game.dll    1BC4EA529B02FB18A6B165F8D668155865B4F49577A53DAE3DACEAF34F711EEE
D2Common.dll  517E5E64D9F641DF2E9112A9ED0A01E868911FF597CE9C02B28808E62C0091E3
Fog.dll       6CD7108A415A02BF1F77486669DC3A8002900E5E80559793C187BD1C5C200799
Storm.dll     83EC65E93A66ED8F31787D74939D8A38323166081BD3564204B7AB776879F2E2
d2server.dll  C6E208D4630F9E7F772B647D4D77E9E7F530B8CE4CC82F0CD8A9CD7CEA01D479
```

The source `include/d2gelib/d2server.dll` hash is
`292FC456864841A5A0139506BD904CB8E61FEC0E2F8B4B8D259291FD1BE3F21B`.
The build changes its `.data` section to executable/read/write, producing the
staged hash listed above.

## Observed boundary

With GE patching disabled, all game DLLs load and `D2GSInit` begins, but the
process eventually overflows the stack in Fog's internal `_chkstk` helper at
RVA `0x11E64`.

A first-chance WinDbg dump resolved the call chain to Fog ordinals `10019`,
`10142`, `10051`, `10023`, and `10028`. Ordinal `10142` initializes the global
memory pool's critical section. Current Windows stores the valid sentinel
`0xFFFFFFFF` in `RTL_CRITICAL_SECTION.DebugInfo`; Fog's 2001 validator at RVA
`0xC791` rejects that sentinel and enters an old fatal-log path whose stack
probe produces the observed overflow.

The classic adapter verifies the exact Fog SHA-256 and all 23 original bytes,
then adjusts that validator to accept only `0xFFFFFFFF` while retaining its
null, alignment, and high-address checks for other values.

The next initialization failure was D2Win ordinal `10037` returning false.
Breakpoint tracing showed successful handles for `D2Data.mpq` and
`Patch_D2.mpq`, but null handles for `D2Sfx.mpq` and `D2Speech.mpq`. Those two
archives are required for base-game initialization and must be staged even on
a headless classic server.

After the base archives load, `d2server` calls D2Lang ordinal `10000` with its
expansion-mode argument set to `1`, which requests
`data\\local\\lng\\ENG\\expansionstring.tbl`. Changing only that argument to
`0` selects the available base string tables. The adapter verifies the
post-build `d2server.dll` SHA-256 and the four call-site bytes at RVA `0x4739`
before changing `push edi` to `push esi`. A debugger trial with that argument
set to zero completed engine initialization and entered the main server loop.

The only enabled initialization descriptor expects a D2Game function at RVA
`0x1EF40`. Its pattern resolves uniquely in classic 1.09 at RVA `0x1EC60`, a
shift of `-0x2E0`, without modifying D2Game code.

The classic Fog pool pointer is `0x6FF780BC`; the 1.09d patch descriptors use
`0x6FF7A4CC`. Classic pattern targets for the Fog pool patch are:

```text
8150 8812 8A36 8D10 8F97 909D 90D7 9173 91D1 933A 937B
90B9 915E 9184
```

Changing only the shared pointer template in a disposable d2server copy and
enabling the complete 1.09d patch set is not safe: initialization crashes
while resolving the next incompatible descriptor. The full descriptor table
must be audited and represented as a hash-gated classic adapter rather than
partially applying 1.09d patches.

## Callback ABI

The callback table layout is stable, but callback signatures are not. Local
disassembly of this exact `D2Game.dll` shows both `fpLeaveGame` dispatch sites,
at `0x6FC32BD5` and `0x6FC32C80`, pushing 12 stack arguments after assigning
the first two fastcall arguments. The callback must therefore return with
`ret 0x30`. The original 15-parameter callback returned with `ret 0x34`, moved
the engine stack four bytes too far, and caused a delayed call to address
`0x00000001` after the final player left.

The 1.09b slot at callback-table offset `0x30` also takes no stack arguments.
The original three-parameter reserved callback returned with `ret 4`, so the
1.09b profile installs a two-parameter fastcall entry for that slot as well.

`D2GS_CALLBACK_ABI_109B` now selects these two measured entries. The existing
1.09d entries remain unchanged. Selection occurs only after the adapter has
verified the exact SHA-256 hashes of `D2Game.dll`, `Fog.dll`, and the staged
`d2server.dll`.

## Safety gate

Normal classic startup is refused before entering the incompatible engine.
Reverse-engineering runs must opt in explicitly:

```powershell
$env:D2GS_EXPERIMENTAL_CLASSIC_109 = "1"
```

This mode always disables the built-in 1.09d GE patch set. It is expected to
remain isolated from player traffic until client join and character transfer
have been tested. Engine startup completes through the listening socket and
main message loop.

## Realm integration

The current PvPGN D2CS protocol extends the original D2GS wire layout with an
authentication signature length, reply signature fields, a server game flag,
and a create-game ladder byte. Without those fields, D2GS reads the zero
signature length as an empty realm name and silently drops the authentication
request. The local protocol structures now match the current PvPGN layout;
signature fields are zeroed because PvPGN has signature verification disabled.

The tested realm stack now completes these transitions:

```text
D2DBS accepts and classifies D2GS
D2CS authenticates and activates D2GS
D2CS accepts maxgame=100
PvPGN authenticates D2CS and activates the realm
classic normal softcore game creation succeeds
an empty game closes automatically after 60 seconds
client token validation and character loading succeed
the client enters the game and receives ACTINITDONE
character save, ladder update, charinfo write, and unlock succeed
the final player can leave and return to the channel
the empty game closes after 60 seconds while D2GS remains active
```

The post-fix lifecycle test on September 18, 2026 loaded the existing 1,016-byte
save with checksum `8BA8FCD2`, saved checksum `E4E2CF24`, and persisted SHA-256
`BA8CB260FFE1E5B234E4B2E8FDDAAB93DA30D5C847FC315BB4BB7EB3BBC047B7`.
`LeaveGame`, `SrvEndGame`, `CloseGame`, and `SrvFreeGame` all completed, and
D2GS remained active and listening on port 4000.

## Classic 1.00 network ingress

The `d2server.dll` resolver table stores D2Net ordinal `10003` in slot
`0x68006D3C`. The calls at `0x68004815` and `0x68004834` initialize the
listener with two arguments. Slot `0x68006D80`, called later without arguments,
is D2Game ordinal `10046`; it performs general game-engine initialization and
is not the D2Net listener entry point.

D2Net 1.00 ordinal `10003` delegates listener creation to Fog ordinal `10114`.
When `D2GSINFO.bIsNT` is true, `d2server` passes a zero mode argument and Fog
selects its legacy IO-completion-port path. The listening socket is created on
current Windows, but accepted data never reaches D2Net's parser or receive
queues. Passing mode `1` selects Fog's alternate socket path. A replay of the
captured 32-byte 1.00 join then reached the accept callback, parser, and D2Game
control queue; processing continued to the expected invalid-game rejection for
the stale replayed game ID.

The exact-hash classic 1.00 profile therefore clears `D2GSINFO.bIsNT` before
starting `d2server`, regardless of the shared `EnableNTMode` registry value.
Other profiles retain the configured value, preserving the verified 1.09
network path.

## Classic 1.00 character transfer

The `d2server` database-character wrapper at `0x68004A40` forwards seven stack
arguments to D2Game ordinal `10007`. The 1.09 implementation accepts seven and
returns with `ret 0x1C`, but the 1.00 implementation at RVA `0x56D0` accepts
only the first five and returns with `ret 0x14`. Its two unconsumed arguments
cause the wrapper to use the seventh argument, `LPPLAYERINFO`, as its return
address. The resulting access violation executes the stack bytes containing
the diagnostic values `0x00ABCDEF` and `0x00FEDCBA` immediately after D2DBS
successfully returns the character save.

The exact-hash 1.00 profile verifies all six ordinal `10007` return sites at
RVAs `0x5710`, `0x57A2`, `0x57E2`, `0x5819`, `0x584F`, and `0x58A7`, then
changes their cleanup from `ret 0x14` to `ret 0x1C`. The two compatibility
arguments remain ignored by D2Game 1.00, while its exported ABI now matches the
unchanged `d2server` wrapper. The 1.09 profile is not modified.

## Classic 1.00 validated lifecycle

The end-to-end test on September 18, 2026 verified the complete path with the
unmodified 1.00 client. PvPGN returned eight saved characters with correct
classic portraits, `baba` completed the realm handoff, and D2GS loaded its
130-byte legacy save and entered game `Foo`.

On normal leave, D2GS invoked save, ladder, leave, charinfo, unlock, and close
callbacks. D2DBS accepted the resulting 873-byte save and 192-byte charinfo,
updated the ladder, and unlocked the character without a checksum error. D2CS
removed the character and destroyed the empty game. The PvPGN-side BNCS,
D2CS, D2DBS, portrait, and legacy-checksum details are recorded in
`docs/diablo2-1.00-compatibility.md` in the companion PvPGN repository.

## Version coverage direction

Supporting 1.00 through 1.09 must be implemented as explicit runtime profiles,
not one broad classic mode. Each supported build needs:

- Exact executable and DLL identities, with hash-gated patches.
- Callback stack-argument counts measured at every dispatch site.
- Version-specific callback entries where signatures differ.
- Required MPQ assets and language mode recorded per build.
- Join, load, save, unlock, final-player exit, and game-close tests.

Measured neighboring versions already demonstrate why this is necessary:
1.06b, 1.07, 1.08, and 1.09b use 12 stack arguments for `fpLeaveGame`, while
1.09d uses 13. The 1.00, 1.01, and 1.02 profiles are lifecycle validated.
Versions 1.03-1.05 remain unmeasured and must not inherit another ABI by
assumption. The current implementation has explicit 1.00, 1.01, 1.02, and
1.09b profiles and preserves the existing 1.09d profile; it does not yet claim
support for the rest of the range.

The 1.01 analysis, implementation details, and complete lifecycle validation
are recorded in `docs/classic-1.01-profile.md`.
The 1.02 profile and complete lifecycle validation are recorded in
`docs/classic-1.02-static-assessment.md`.
