# Classic 1.04b/1.04c D2GS Profile

This document records the statically measured Diablo II Classic 1.04b and
1.04c D2GS profile. Both releases contain byte-identical engine DLLs, so one
exact-hash profile supports both clients. All addresses and original bytes
apply only to the identities below, and the adapter fails closed when an
identity or byte sequence differs.

The profile is disabled unless `D2GS_EXPERIMENTAL_CLASSIC_104=1` is present.
The static profile has not yet completed live lifecycle validation.

## Binary Identity

```text
D2Game.dll    6B4BBE026D3E941499A3F2074906017D3084D86295BBDB0E7A983DD68037707D
D2Client.dll  64B3A5FB13F282BE593317156BC1698EF878F00CF632A9021E3BAEAF57E2549B
D2Common.dll  A3FEB017B5573924D4D614F0A5B47FA3AAA19E7563A8E3BF06509BE8BC1661A7
D2Net.dll     BEE5D7A98FC298FFA6A6DEB44BD198098F544E1732125AEC3514D8B00D574F76
D2Win.dll     AD2270ABB74432DCE0C1E4396DA970A5CB8555557E9E3FBC9919D873668E68D3
Fog.dll       979D660C0FB7316ACF036D752A71555671C7A20092E445CC1A4F5EEC6D5918CD
d2server.dll  C6E208D4630F9E7F772B647D4D77E9E7F530B8CE4CC82F0CD8A9CD7CEA01D479
```

The profile reuses the validated early `d2server.dll` host substitutions. Its
version-specific values are:

| Field | 1.04b/1.04c value |
|---|---|
| D2Client descriptor immediate | RVA `0xBD2C`, bytes `2C BD 00 00` |
| D2Client singleton pointer | `0x6FC42F10`, bytes `10 2F C4 6F` |
| D2Game callback-table global | `0x6FD7475C` |
| D2Game ordinal `10007` body | RVA `0x59F0` |
| D2Game ordinal `10046` body | RVA `0x5030` |
| Callback ABI | `D2GS_CALLBACK_ABI_104` |

The D2Client initialization sequence is:

```text
RVA 0xBD26: 68 60 30 C4 6F    push 0x6FC43060
RVA 0xBD2B: 68 10 2F C4 6F    push 0x6FC42F10
```

As in 1.01 through 1.03, the host descriptor replacement is the RVA of the
second immediate operand, while the second pushed absolute value is the
singleton base. The fixed host sites receive:

```text
d2server RVA 0x2AB4: E1 81 00 00 -> 2C BD 00 00
d2server RVA 0x3194: 88 0A BB 6F -> 10 2F C4 6F
```

## Fog Diagnostic

Fog ordinal `10019` occupies RVA `0xA350`. Its terminal pre-initialization
diagnostic is structurally equivalent to the startup-ordering hazard measured
for 1.01 through 1.03:

```text
Fog RVA 0xA5EA: E8 C1 FB FF FF    call Fog RVA 0xA1B0
```

The exact-hash profile verifies all five bytes and replaces only this call:

```text
E8 C1 FB FF FF -> 90 90 90 90 90
```

Normal Fog initialization and synchronization remain active.

## Database Export

D2Game ordinal `10007` has six `ret 0x14` paths. The early host supplies two
additional compatibility arguments, so every measured path is changed to
`ret 0x1C` after all original bytes match:

```text
RVA 0x5A39: C2 14 00 -> C2 1C 00
RVA 0x5B12: C2 14 00 -> C2 1C 00
RVA 0x5BB3: C2 14 00 -> C2 1C 00
RVA 0x5C3B: C2 14 00 -> C2 1C 00
RVA 0x5C9A: C2 14 00 -> C2 1C 00
RVA 0x5CF1: C2 14 00 -> C2 1C 00
```

## Callback ABI

The 1.04 callback table is not equivalent to the 1.01 through 1.03 table. It
combines early and later signatures, requiring a distinct ABI selection:

| Slot | Callback | Total arguments | Stack arguments | Adapter |
|---:|---|---:|---:|---|
| `0x04` | LeaveGame | 10 | 8 | `LeaveGame104` |
| `0x08` | GetDatabaseCharacter | 3 | 1 | `GetDatabaseCharacter104` |
| `0x0C` | SaveDatabaseCharacter | 6 | 4 | Existing modern wrapper |
| `0x20` | UnlockDatabaseCharacter | 2 | 0 | `UnlockDatabaseCharacter104` |
| `0x28` | UpdateCharacterLadder | 6 | 4 | `UpdateCharacterLadder100` |
| `0x30` | Reserved2 | 3 | 1 | Existing three-argument wrapper |

`LeaveGame104`, `GetDatabaseCharacter104`, and
`UnlockDatabaseCharacter104` recover the account name from the server's
character state because 1.04 omits it. The leave wrapper preserves the supplied
game and player data. The unlock wrapper performs the D2DBS unlock rather than
using the later callback that only logs the event.

Measured callback dispatch RVAs are:

```text
LeaveGame:               0x275B, 0x28BE
GetDatabaseCharacter:    0x60CA, 0x6B93
SaveDatabaseCharacter:   0x4739A
UnlockDatabaseCharacter: 0x6A10
UpdateCharacterLadder:   0x6D9E
Reserved2:               0x7721
```

The profile also clears `D2GSINFO.bIsNT`, selecting the established early
listener path. It does not patch D2Net parser code.

## Validation Status

The exact hashes, host values, Fog call, database returns, and callback
dispatch signatures have been statically verified for both supplied patch
directories. The implementation builds as Release x86 with warnings treated as
errors. Live startup and lifecycle testing remain required. The companion BNCS
identity and protocol notes are in `docs/diablo2-1.04-protocol.md` in the PvPGN
repository.
