# Classic 1.01 D2GS Profile

This document records the static binary analysis and runtime validation for the
exact-hash Diablo II 1.01 D2GS profile. Engine startup and the complete
live-client lifecycle are validated. Every address below refers to the analyzed
binary set and must be used only after all relevant SHA-256 identities and
original bytes match.

The PvPGN-side BNCS, MCP, ladder, lobby, and D2Net wire formats are documented
in `docs/diablo2-1.00-1.01-protocol.md` in the companion PvPGN repository.

## Binary identity

The analyzed client installation is `D2DV_101`. Its exact engine identities
are:

```text
D2Game.dll    BA2A573C7E802F1B5BA8209B701F7DEE0D956A16FE0C9E6F7282DF18BBAC8756
D2Client.dll  1F7799A04E15C2C7B2028EF79B0ECF81640D075CB745BB5A03F9261EBBF67A68
D2Common.dll  EE2C05C8B0671881F36903BE3D553D94E0799B613220AF2C5F1B161F824C83FD
D2Net.dll     33CD7570450DD582EA5A7077B63572D82DF44F368828A5E19650B9E66D81B8C5
D2Win.dll     B6A928E4A529D55D3778A0CDCD8096F1AB24C1F3C603F1CBBF240415A997790D
Fog.dll       E3DE583CDFE7A62983DF8B7DB14FB95BE68FE5B36C7E995B485368FEAB626566
d2server.dll  C6E208D4630F9E7F772B647D4D77E9E7F530B8CE4CC82F0CD8A9CD7CEA01D479
```

`D2Net.dll` is part of the exact runtime identity gate because its network ABI
is version-specific. The staged `d2server.dll` is the same post-build host
already used by the 1.00 profile.

## Host resolver and setup patches

The identical staged host means the expected d2server bytes and host RVAs are
unchanged. The replacement ordinals below are specific to the analyzed 1.01
DLL set.

| d2server RVA | Purpose | Expected | Replacement |
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
| `0x47B1` | D2Common caller cleanup | `89 45 F8` | `83 C4 0C` |
| `0x436A` | Init event pointer source | `FF 76 14` | `FF 70 14` |

D2Win ordinal `10033` replaces the host's expected ordinal `10037`.

The Fog remaps are:

```text
10021 -> 10019    log setup
10101 -> 10074    subsystem initialization
10019 -> 10017    global initialization
10089 -> 10065    pool
10218 -> 10017    statistics fallback; call site disabled
10185 -> 10017    memory fallback; call site disabled
```

D2Common ordinal `10554` is exported at RVA `0x29C3` and jumps to body RVA
`0xCB70`. That body closely matches the 1.00 implementation at RVA `0xCB60`
and uses caller cleanup for three stack arguments. The host must therefore use
`add esp,0x0C` after the call.

`D2GSINFO.hEventInited` is at offset `0x14`. The init-event patch changes the
saved-register source to the canonical structure pointer.

## Fog pre-initialization diagnostic

Runtime tracing found a 1.01-only ordering hazard after the resolver remaps.
The host calls Fog ordinal `10019` at d2server RVA `0x46B0`, then calls global
initializer ordinal `10017` at RVA `0x46CF`. Fog 1.01 ordinal `10019` emits a
diagnostic before returning:

```text
Fog RVA 0xD2D2: call Fog RVA 0x123A
Fog RVA 0xCDC1: call Fog ordinal 10026
Fog RVA 0xE8CD: push Fog RVA 0xAE3E8
Fog RVA 0xE8D2: EnterCriticalSection
```

Ordinal `10017` initializes that critical section later at Fog RVA `0xD3D1`.
The premature diagnostic therefore faults in `ntdll` while writing through a
null `RTL_CRITICAL_SECTION.DebugInfo`. Fog 1.00 does not make this diagnostic
call from ordinal `10019`, so its startup is unaffected.

The exact-hash 1.01 profile suppresses only the premature diagnostic call:

```text
Fog RVA 0xD2D2: E8 63 3F FF FF -> 90 90 90 90 90
```

The adapter verifies all five original bytes before patching. It does not
disable critical-section synchronization or initialize the structure itself;
the normal ordinal `10017` path remains responsible for initialization.

## D2Client descriptor

The 1.01 D2Client initialization sequence is:

```text
RVA 0x15BD6: push 0x1012EC20
RVA 0x15BDB: push 0x1012EAD0
```

The second immediate begins at RVA `0x15BDC`. The host patches are:

| d2server RVA | Expected | Replacement |
|---:|---|---|
| `0x2AB4` | `E1 81 00 00` | `DC 5B 01 00` |
| `0x3194` | `88 0A BB 6F` | `D0 EA 12 10` |

Profile values:

```text
D2Client descriptor target RVA: 0x15BDC
D2Client singleton pointer:      0x1012EAD0
```

## Database-character export

D2Game ordinal `10007` begins at RVA `0x5750`. The host wrapper passes seven
stack arguments, while this implementation consumes five and returns with
`ret 0x14`. As with 1.00, each return must retain the two compatibility
arguments by changing the cleanup to `ret 0x1C`.

The five 1.01 return sites are:

```text
RVA 0x5790: C2 14 00 -> C2 1C 00
RVA 0x5822: C2 14 00 -> C2 1C 00
RVA 0x5862: C2 14 00 -> C2 1C 00
RVA 0x5899: C2 14 00 -> C2 1C 00
RVA 0x5924: C2 14 00 -> C2 1C 00
```

Unlike 1.00, 1.01 has five return paths rather than six. A profile must not
reuse the 1.00 return-RVA table.

## D2Net ingress

D2Game imports the two receive queues at:

```text
Control receive, D2Net ordinal 10011: D2Game IAT RVA 0xA4AC8
Game receive, D2Net ordinal 10010:    D2Game IAT RVA 0xA4ACC
```

The recovered D2Net functions are:

| Function | Thunk RVA | Original thunk bytes | Body RVA | Convention |
|---|---:|---|---:|---|
| Parser | `0x100A` | `E9 D1 11 00 00` | `0x21E0` | fastcall, 2 register + 6 stack arguments |
| Accept | `0x104B` | `E9 80 12 00 00` | `0x22D0` | fastcall, 2 register arguments |
| Delivery | `0x10A5` | `E9 16 11 00 00` | `0x21C0` | fastcall, 2 register + 1 stack argument |

The parser body begins:

```text
83 FA 04 56 73 09
```

The fourth byte differs from 1.00 because 1.01 saves `ESI` rather than `EBX`.
Temporary validation instrumentation verified the exact six bytes before
installing its trampoline. That instrumentation was removed after lifecycle
validation; the production profile does not patch live D2Net code.

D2Net ordinal `10003` is exported at RVA `0x107D` and delegates through body
RVA `0x23F0` to Fog ordinal `10114`. The Fog implementation is equivalent to
the 1.00 network-mode decision, shifted by `+0xC0`. RVA `0x36F1` selects the
legacy NT path only when the forwarded mode is zero and Windows identifies as
`VER_PLATFORM_WIN32_NT`.

The legacy IOCP path creates a listening socket on current Windows but did not
deliver accepted data in the validated 1.00 run. The 1.01 profile sets
`D2GSINFO.bIsNT` to `FALSE`, selecting the alternate socket path. The live 1.01
test delivered control and game packets through that path.

## Callback ABI

The 1.01 D2Game callback table global is `0x100BE6CC`; the associated control
global is `0x100BE74C`. Static dispatch-site analysis gives these signatures:

| Slot | Callback | Total arguments | Stack arguments | Required adapter |
|---:|---|---:|---:|---|
| `0x04` | LeaveGame | 8 | 6 | `LeaveGame100` |
| `0x08` | GetDatabaseCharacter | 2 | 0 | `GetDatabaseCharacter100` |
| `0x0C` | SaveDatabaseCharacter | 5 | 3 | `SaveDatabaseCharacter100` |
| `0x20` | UnlockDatabaseCharacter | 2 | 0 | `UnlockDatabaseCharacter100` |
| `0x28` | UpdateCharacterLadder | 6 | 4 | `UpdateCharacterLadder100` |
| `0x30` | Reserved2 | 2 | 0 | `ReservedCallback2_109b` |

The 1.00 callback profile cannot be reused unchanged. Its reserved slot uses
the three-argument callback that returns with `ret 4`; 1.01 invokes slot
`0x30` with no stack arguments. Reusing that entry would move the engine stack
by four bytes and cause delayed corruption. The implementation exposes a
distinct `D2GS_CALLBACK_ABI_101`, even though most entries reuse measured 1.00
adapters.

## Required safety gate

The profile must fail closed unless all of these conditions hold:

- Every listed DLL and staged d2server hash matches.
- Every listed DLL identity and every patched host, Fog, and D2Game byte
  sequence matches.
- Built-in 1.09d GE patching is disabled.
- A dedicated experimental 1.01 opt-in is present.
- `D2GSINFO.bIsNT` is forced to `FALSE` for the measured 1.00 and 1.01 early
  profiles only.
- The 1.01 callback ABI is selected explicitly.

No address or calling convention in this document should be generalized to an
unmeasured patch version.

## Runtime validation

The September 19, 2026 startup test verified that the exact profile reaches the
main loop, initializes MPQs, language, game tables, the alternate network
listener, and callbacks, authenticates with D2DBS and D2CS, and polls both
receive queues. A 1.00 startup regression passed with the same executable.

With temporary D2Net tracing enabled, a replay of the documented shared 32-byte
join frame to the 1.01 listener was accepted, parsed, and delivered as a
28-byte payload to D2Game's control queue. This validated the alternate
listener path and control receive queue. The stale 1.00 game ID and token could
not validate a real 1.01 game handoff.

The live-client test on September 19, 2026 then verified:

- PvPGN matched `Game.exe` version `1.0.0.1` to `D2DV_101`, authenticated the
  account, displayed the `101` lobby tag, and sent the two-character early
  character list.
- The early MCP login, character authentication, game creation, and realm
  handoff completed.
- The first 32-byte join validated game ID `2` and token `0x6AAEA03C` for
  character `zombie`; control and game packets reached both receive queues.
- D2Game loaded the original 130-byte character and entered game `Foo`.
- Normal leave saved an 846-byte version-`0x47` legacy character with SHA-256
  `437AA35CC6EFB78D3BFAB0B454615FBB755867B4A00940B3D8F1DEC96E0F1CE6`,
  updated ladder and charinfo, unlocked the character, returned to the channel,
  and closed the empty game.
- A second login selected the saved character, validated game ID `4` and token
  `0x6AAEDEE7`, loaded all 846 bytes, entered game `Ljk`, and completed the same
  save, ladder, charinfo, unlock, leave, and close sequence.
- The second save remained 846-byte version `0x47`; its SHA-256 was
  `A31CC15AB3F04DAE8C4F21175680D6F9AD8FC3A3AB18F8BD10CD9AC6A0AF8D99`.
  D2DBS retained the first validated save as the backup.
- D2GS remained active after both cycles, with no exception, checksum failure,
  callback stack fault, or locked character.

## Implementation map

- `sources/classicadapter.c` owns exact identities, expected-byte descriptors,
  ordinal remaps, and engine patches. Required post-load D2Game patches fail
  startup closed before D2GS is advertised to the realm.
- `sources/d2ge.c` applies the selected compatibility profile and network mode.
- `sources/callback.c` provides the measured callback ABI adapters.
- `include/d2gelib/d2server.dll` is the source host binary transformed into the
  staged hash listed above by the build.
