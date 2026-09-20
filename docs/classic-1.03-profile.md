# Classic 1.03 D2GS Profile

This document records the exact-hash Diablo II Classic 1.03 D2GS profile and
its runtime validation. All addresses and original bytes apply only to the
binary identities listed below. The adapter fails closed when an identity or
byte sequence differs.

## Binary identity

```text
D2Game.dll    B5AB82EBAD21AD08BC2FA549778BD22AAE653610630DBF5112AF3B0123277CFB
D2Client.dll  B749A4D20ED3C6AB30F12C0474F9A3AE52A079B65B16382FCEB6E3BF975D06A4
D2Common.dll  BD4E40B21828AEC3F89F9D6E9D332EEC8A77E2E0683F621046089F640BBD4E1E
D2Net.dll     0BDA381E0C060733EB34216625ACDD2EBB472795F184596D57CEEE81DF3F1764
D2Win.dll     591D75302A475115A8B62041408F1894728ADF103E5979E192F77B31BBD67C41
Fog.dll       073EB5F746E3FA55214284A2E0FA542D69E14A255ECFEA0690B5899B3D9A2E20
d2server.dll  C6E208D4630F9E7F772B647D4D77E9E7F530B8CE4CC82F0CD8A9CD7CEA01D479
```

The profile is disabled unless `D2GS_EXPERIMENTAL_CLASSIC_103=1` is present.
Without the variable, the exact 1.03 engine set is detected and rejected before
engine initialization.

## Profile values

The staged `d2server.dll` is the same early host used by the 1.00 through 1.02
profiles. The shared host patch sites and expected bytes therefore remain
unchanged. The 1.03-specific values are:

| Field | 1.03 value |
|---|---|
| D2Client descriptor immediate | `AC 55 01 00` |
| D2Client singleton pointer | `90 E9 12 10` |
| Callback ABI | `D2GS_CALLBACK_ABI_101` |
| Fog pre-init diagnostic RVA | `0xFAA8` |
| Fog expected bytes | `E8 A1 17 FF FF` |

The Fog call is replaced by five NOP bytes only after all original bytes match.
It is the same narrowly scoped pre-initialization diagnostic mitigation measured
for 1.01 and 1.02; normal Fog initialization remains active.

## Database-character export

D2Game ordinal `10007` has five `ret 0x14` paths. The host supplies two
additional compatibility arguments, so the exact profile changes each return
to `ret 0x1C`:

```text
RVA 0x5E64: C2 14 00 -> C2 1C 00
RVA 0x5EF4: C2 14 00 -> C2 1C 00
RVA 0x5F34: C2 14 00 -> C2 1C 00
RVA 0x5F6B: C2 14 00 -> C2 1C 00
RVA 0x600B: C2 14 00 -> C2 1C 00
```

The callback signatures match the measured 1.01/1.02 ABI, so the profile uses
the existing `D2GS_CALLBACK_ABI_101` wrappers rather than introducing inferred
wrappers.

## D2Net ingress

The 1.03 engine uses the established early listener path. The production
profile does not patch live D2Net parser code. Runtime validation reached
D2Net initialization, callback registration, the D2CS/D2DBS handshakes, client
token validation, and character entry.

## Lifecycle validation

The exact profile was built as Release x86 with warnings treated as errors.
Fresh startup regressions also passed for the 1.00, 1.01, and 1.02 profiles.

The live 1.03 lifecycle created `bonneezz` as a 130-byte revision-`0x47`
Necromancer save. The first game expanded it to 846 bytes. A second game loaded
and saved the complete 846-byte file. D2DBS completed the ladder update,
charinfo persistence, backup rotation, unlock, and game close in both cycles.

The companion PvPGN protocol and identity notes are in
`docs/diablo2-1.03-protocol.md`.
