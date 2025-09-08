## FBA - Final Burn Alpha v0.2.96.86 for MiyooCFW (base of fba-a320)

ROMSETs via MAME 0.126

### Cross-Compile (MiyooCFW):

- compile binary:
```
make -j$(nproc)
```
- or create IPK package:
```
make -j$(nproc) ipk
```
the current release comes with old QSound emu, 
if you want original audio in CP System then use `QSOUND_FBNEO=1` make flag

### Native build (linux):

```
no instructions yet
```
