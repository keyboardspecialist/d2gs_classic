# Classic 1.02 Compatibility Assessment

This document records the static compatibility assessment and runtime
validation for the exact Diablo II 1.02 binary set. It covers the PvPGN
BNCS/MCP/D2DBS path and the D2GS engine adapter. The complete live-client
lifecycle is validated.

All addresses below apply only to the hashes listed here. The implementation
must fail closed if an identity or original byte sequence differs.

## Binary identity

The analyzed installation is `D2DV_102` and reports `Game.exe` version
`1.0.2.0`. Its identities are:

```text
Game.exe       EE1DD916A917A43C15361FE2B887AFF21C4DAF9FD8C8AF39E05318B0AB4099D7
D2Game.dll     783FF9BF1C1EADB40362F81D43D1FFAE7D3BDA7F47FC9B2E2DB4A3E23A44F1BF
D2Client.dll   F4CC816C9C863F77A529B19366623A2F6DD9981598C7C3E61366509E44DCD6B9
D2Common.dll   B08B185DF8B95395A214CAC2FC64AA3A8758535EA22501D8CFE7C4FC44DC5B9E
D2Net.dll      6198AEE4890AFE8872219AA67BD42DE839CB43067E8A9705EF671A8EC22805C9
D2Win.dll      5A6C4A00CD12AC273EA0BEA916A20908B6B7FF60C6936EE1783F58869B9E6189
Fog.dll        5899410C9E4C3DE561B507C437CAE40494683294A70C218E75F3F3AFC31159A7
D2MCPClient.dll FA74B959706BA24A21C00FEA9E47DCBE4DFF4834EED35FC24D57F9B016B9A748
D2Multi.dll    A572DF88F2084B1E531B7262945D2290558E8D2B69F811EA648371162F35FDF7
Bnclient.dll   12286F43A32E7A9CDB57E7E0E9AA57EA5494CABB76B24734BE0DF28416F9B84D
Patch_D2.mpq   EE7EC68933B1A662ADC307FBDA6B516EB2F6E1F88E3CCB3E7AD90F2B4DFAE3A1
d2server.dll   C6E208D4630F9E7F772B647D4D77E9E7F530B8CE4CC82F0CD8A9CD7CEA01D479
```

`Game.exe` is 346243 bytes and has PE timestamp `0x396659E0`. The filesystem
last-write timestamp in the archive is from 2021 and is not canonical release
metadata.

## PvPGN compatibility

### BNCS version selection

The embedded version and surrounding patch-version scheme indicate `D2DV`
version ID `2`. Live authentication reported version `1.0.2.0`, version ID
`0x00000002`, and matched `D2DV_102`. PvPGN's
`conn_is_legacy_d2_client()` explicitly includes IDs `0`, `1`, and `2`. Its
consumers select:

- The early SID `0x35` realm handoff.
- The early SID `0x37` character enumeration.
- The early lobby/chat message class and portrait conversion.

This should remain a capability predicate rather than a broad numerical
version range. Every accepted patch must be named explicitly.

The version DWORD is `0x01000200`, corresponding to `1.0.2.0`. Applying
PvPGN's `IX86ver1.mpq` seed and configured equation to `Game.exe`,
`Bnclient.dll`, and `D2Client.dll` produces CheckRevision checksum
`0x0BA6BBCD`. The calculation was independently reproduced against the exact
hashes above and is configured under `D2DV_102`.

The configured `fileMetadata` value, `Game.exe 05/31/21 11:23:26 346243`,
records the supplied file but contains the archive's noncanonical 2021
filesystem timestamp. PvPGN does not read this field when loading the JSON and
does not use it to select or accept a version. The client reported
`Game.exe 05/31/21 11:23:26 45056`; neither archive value can recover the
original retail filesystem timestamp.

### MCP and realm behavior

The 1.01 and 1.02 `D2MCPClient.dll` packet construction code is byte-identical
apart from eight PE timestamp/checksum bytes. The analyzed D2Client join path
uses the same 32-byte client frame and 28-byte D2Game payload documented for
1.00 and 1.01.

D2CS does not receive the BNCS version ID. It recognizes the 51-byte early
login request and stores that as `legacy_100`; the flag already selects the
early character-list, ladder, and portrait paths. No new D2CS packet layout is
indicated by the 1.02 binaries.

### Character saves

D2DBS compatibility is save-format driven rather than BNCS-version driven.
Files with save version below `D2CHARSAVE_CHECKSUM_MIN_VERSION` (`0x5C`) use
the legacy offsets and skip the later checksum requirement. D2Game 1.02 rejects
PvPGN's shared revision-`0x59` newbie template before `EnterGame`. D2CS now
writes revision `0x47` when creating a character for a connection using the
early MCP layout, without changing the template for later clients.

Runtime validation created a 130-byte revision-`0x47` Amazon, which D2Game
expanded to 873 bytes on its first save. A second game loaded and saved the
873-byte file successfully. D2DBS retained revision `0x47` and created both
charsave and charinfo backups.

## D2GS static profile

The 1.02 engine requires its own exact-hash `CLASSICEARLYPROFILE` and an
independent opt-in such as `D2GS_EXPERIMENTAL_CLASSIC_102`. The measured
callback signatures are identical to `D2GS_CALLBACK_ABI_101`, so a new set of
callback wrappers is not required. Reusing that ABI selection is supported by
measurement, not by an assumed version range.

The profile-specific values are:

| Field | 1.02 value |
|---|---|
| D2Client descriptor immediate | RVA `0x1550C`, bytes `0C 55 01 00` |
| D2Client singleton | `0x1012EA30`, bytes `30 EA 12 10` |
| D2Game callback-table global | `0x100BF84C` |
| D2Game ordinal `10007` body | RVA `0x57B0` |
| Callback ABI | Measured equivalent to `D2GS_CALLBACK_ABI_101` |

The D2Client initialization sequence includes:

```text
RVA 0x15506: push 0x1012EB80
RVA 0x1550B: push 0x1012EA30
```

The second immediate starts at RVA `0x1550C`. The unchanged staged host still
expects its original bytes at RVAs `0x2AB4` and `0x3194`; only the two
replacement values are profile-specific.

## Host resolver and setup patches

The staged `d2server.dll` is identical to the validated 1.00/1.01 host. All
host RVAs and expected bytes remain unchanged. Static comparison confirms that
the 1.02 exports implement the same selected early interfaces.

| d2server RVA | Purpose | Expected | 1.02 replacement |
|---:|---|---|---|
| `0x4739` | Base language mode | `59 57 33 C9` | `59 56 33 C9` |
| `0x3BC0` | D2Win MPQ ordinal | `35 27 00 00` | `31 27 00 00` |
| `0x3C08` | Fog log ordinal | `25 27 00 00` | `23 27 00 00` |
| `0x3C14` | Fog subsystem-init ordinal | `75 27 00 00` | `5A 27 00 00` |
| `0x3C20` | Fog global-init ordinal | `23 27 00 00` | `21 27 00 00` |
| `0x3C2C` | Fog pool ordinal | `69 27 00 00` | `51 27 00 00` |
| `0x3C38` | Fog statistics ordinal | `EA 27 00 00` | `21 27 00 00` |
| `0x3CBC` | Fog memory ordinal | `C9 27 00 00` | `21 27 00 00` |
| `0x46F0` | Disable optional Fog statistics call | `FF 15 88 6D 00 68` | `33 C0 90 90 90 90` |
| `0x480A` | Disable optional Fog memory call | `FF 15 A0 6D 00 68` | `33 C0 90 90 90 90` |
| `0x3CD4` | D2Common loader ordinal | `50 29 00 00` | `3A 29 00 00` |
| `0x47B1` | Existing early loader cleanup patch | `89 45 F8` | `83 C4 0C` |
| `0x436A` | Init event pointer source | `FF 76 14` | `FF 70 14` |
| `0x2AB4` | D2Client descriptor RVA | `E1 81 00 00` | `0C 55 01 00` |
| `0x3194` | D2Client singleton pointer | `88 0A BB 6F` | `30 EA 12 10` |

The ordinal substitutions are unchanged:

```text
D2Win 10037 -> 10033

Fog 10021 -> 10019    log setup
Fog 10101 -> 10074    subsystem initialization
Fog 10019 -> 10017    global initialization
Fog 10089 -> 10065    pool
Fog 10218 -> 10017    statistics fallback; call site disabled
Fog 10185 -> 10017    memory fallback; call site disabled

D2Common 10576 -> 10554
```

Selected 1.02 export and body RVAs are:

| DLL/ordinal | Export RVA | Body RVA |
|---|---:|---:|
| D2Win `10033` | `0x1465` | `0x63B0` |
| D2Win `10037` | `0x1285` | `0x6320` |
| Fog `10017` | `0x1235` | `0xE1F0` |
| Fog `10019` | `0x1208` | `0xDE90` |
| Fog `10021` | `0x1262` | `0xEDC0` |
| Fog `10065` | `0x10DC` | `0xB560` |
| Fog `10074` | `0x12C6` | `0x11390` |
| Fog `10089` | `0x1253` | `0x26D0` |
| Fog `10101` | `0x1136` | `0x2E90` |
| D2Common `10554` | `0x29B4` | `0xCB80` |
| D2Common `10576` | `0x169A` | `0x13E90` |

The 1.02 D2Common `10554` body aligns with the 1.01 implementation shifted by
`+0x10` through its prologue and epilogue. It ends at RVA `0xDDB5` with
`C2 0C 00`. The existing host cleanup patch is part of the already validated
early-host sequence and should be preserved unless that surrounding host stack
sequence is separately re-audited.

## Fog pre-initialization diagnostic

The 1.01 startup ordering hazard persists at a new address in 1.02. Fog ordinal
`10019` enters body RVA `0xDE90` and emits a diagnostic immediately before it
returns:

```text
Fog RVA 0xE128: E8 1C 31 FF FF    call Fog RVA 0x1249
```

The global initializer selected through ordinal `10017` enters body RVA
`0xE1F0` and initializes the critical section beginning at RVA `0xE211`. The
diagnostic reaches Fog's synchronized logging path before that initializer has
run. The exact-hash 1.02 profile therefore requires the same narrow mitigation
as 1.01 at the new address:

```text
Fog RVA 0xE128: E8 1C 31 FF FF -> 90 90 90 90 90
```

The adapter must verify all five original bytes before replacing them. This
does not disable synchronization or replace normal critical-section
initialization.

## Database-character export

D2Game ordinal `10007` begins at RVA `0x57B0`. It has five `ret 0x14` paths,
matching the five-path 1.01 shape but at different addresses. Each site must
retain the host's two compatibility arguments by changing `ret 0x14` to
`ret 0x1C`:

```text
RVA 0x57F4: C2 14 00 -> C2 1C 00
RVA 0x5881: C2 14 00 -> C2 1C 00
RVA 0x58C1: C2 14 00 -> C2 1C 00
RVA 0x58F8: C2 14 00 -> C2 1C 00
RVA 0x5998: C2 14 00 -> C2 1C 00
```

No return table from another patch may be reused.

## Callback ABI

The 1.02 callback table is stored at `0x100BF84C`. Every version-sensitive
dispatch signature matches the measured 1.01 ABI:

| Slot | Callback | Total arguments | Stack arguments | Adapter |
|---:|---|---:|---:|---|
| `0x04` | LeaveGame | 8 | 6 | `LeaveGame100` |
| `0x08` | GetDatabaseCharacter | 2 | 0 | `GetDatabaseCharacter100` |
| `0x0C` | SaveDatabaseCharacter | 5 | 3 | `SaveDatabaseCharacter100` |
| `0x20` | UnlockDatabaseCharacter | 2 | 0 | `UnlockDatabaseCharacter100` |
| `0x28` | UpdateCharacterLadder | 6 | 4 | `UpdateCharacterLadder100` |
| `0x30` | Reserved2 | 2 | 0 | `ReservedCallback2_109b` |

Selecting `D2GS_CALLBACK_ABI_101` for the distinct 1.02 hash profile is the
minimal correct implementation. It also preserves the measured early-engine
selection of `D2GSINFO.bIsNT = FALSE` without adding duplicate wrappers.

## D2Net ingress

Neither the 1.01 nor 1.02 D2Game imports D2Net ordinal `10003`. Both import the
same 11 D2Net ordinals, in different table order:

```text
10024 10019 10015 10020 10011 10010
10012 10014 10006 10016 10021
```

Ordinal `10003` is resolved by the host, exported at D2Net RVA `0x107D`, and
delegates to listener body RVA `0x23F0`. The relevant 1.01 and 1.02 parser and
listener code is structurally equivalent. Two internal thunks at RVAs
`0x1028` and `0x102D` exchange target placement between the versions without
changing the selected behavior.

The production profile does not patch D2Net code. It hash gates `D2Net.dll`
and selects the alternate non-NT socket mode. Live game joins validated packet
delivery through this path.

## Minimal implementation map

The D2GS implementation contains these bounded changes:

- Add the six exact 1.02 engine hashes and a `Classic102Profile` in
  `sources/classicadapter.c`.
- Add profile-owned Fog pre-init RVA and expected bytes instead of extending
  the current `profile == &Classic101Profile` special case.
- Add the 1.02 descriptor bytes and five database return RVAs.
- Select the measured `D2GS_CALLBACK_ABI_101`; do not add duplicate callback
  wrappers.
- Add `D2GS_EXPERIMENTAL_CLASSIC_102` as an independent fail-closed opt-in.

The PvPGN implementation adds `D2DV_102` to `versioncheck.json.in`, includes
version ID `2` in the explicit early-client capability predicate, lists the
matching commented autoupdate tag, and writes revision `0x47` for new characters
using the early MCP layout. Live authentication confirmed the version ID.

No static evidence calls for a new MCP serializer, D2CS packet structure,
D2DBS save branch, D2Net trampoline, or callback wrapper.

## Runtime validation

On September 19, 2026, the exact-hash profile completed the full lifecycle on
modern Windows:

1. BNCS authenticated version `1.0.2.0`, version ID `0x00000002`, as
   `D2DV_102`; account login, early character enumeration, lobby rendering,
   MCP character login, and realm handoff succeeded.
2. D2GS initialized MPQs, language, data tables, callbacks, and the alternate
   network listener without a Fog critical-section fault. D2CS and D2DBS
   authenticated it and activated the game server.
3. D2CS created `zona` as a 130-byte revision-`0x47` level-1 Amazon. D2GS
   created game `Test`, validated the token, loaded the save, and called
   `EnterGame` with the correct level and class.
4. Normal leave produced an 873-byte revision-`0x47` save, ladder and charinfo
   updates, a clean unlock, and normal game close.
5. Game `Ljl` loaded the complete 873-byte save, entered normally, saved it
   again, updated ladder and charinfo, and unlocked the character.
6. D2DBS backup output retained the preceding 873-byte revision-`0x47` save,
   confirming backup rotation across the second save.

Fresh 1.00 and 1.01 startup regressions passed with the same D2GS executable
before 1.02 was restored as the active runtime. Exact 1.02 is therefore
lifecycle validated for the measured binary set.
