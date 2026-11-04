#!/usr/bin/env python3
"""Package a built Windows bundle and the matching complete project sources."""
from pathlib import Path
import argparse,shutil,zipfile,hashlib,struct
root=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--bundle',type=Path,required=True);args=p.parse_args()
module=args.bundle/'Contents/x86_64-win/VocaBox.vst3'
if not module.is_file() or module.read_bytes()[:2]!=b'MZ':raise SystemExit('Expected a Windows PE VST3 bundle')
binary=module.read_bytes()
pe=struct.unpack_from('<I',binary,0x3c)[0]
if binary[pe:pe+4]!=b'PE\0\0' or struct.unpack_from('<H',binary,pe+4)[0]!=0x8664 or struct.unpack_from('<H',binary,pe+24)[0]!=0x20b:
 raise SystemExit('Expected PE32+/AMD64 VST3')
out=root/'dist';out.mkdir(exist_ok=True)
stage=out/'VocaBox-Windows-x64';stage.mkdir(exist_ok=True)
shutil.copytree(args.bundle,stage/'VocaBox.vst3',dirs_exist_ok=True)
for f in ['README.md','VALIDATION.md','UI_CHANGES.md']:shutil.copy2(root/f,stage/f)
shutil.copytree(root/'THIRD_PARTY',stage/'THIRD_PARTY',dirs_exist_ok=True)
shutil.copytree(root/'Review',stage/'Review',dirs_exist_ok=True)
shutil.copy2(root/'Resources/PRESET_MAP.csv',stage/'PRESET_MAP.csv')
with zipfile.ZipFile(out/'VocaBox-Windows-x64.zip','w',zipfile.ZIP_DEFLATED) as z:
 for f in sorted(stage.rglob('*')):
  if f.is_file():z.write(f,f.relative_to(stage))
with zipfile.ZipFile(out/'VocaBox-Source.zip','w',zipfile.ZIP_DEFLATED) as z:
 for name in ['CMakeLists.txt','.github','.gitignore','README.md','VALIDATION.md','UI_CHANGES.md','Review','voxbox.html','Source','Resources','Tests','scripts','cmake','THIRD_PARTY']:
  path=root/name
  for f in ([path] if path.is_file() else sorted(path.rglob('*'))):
   if f.is_file() and '__pycache__' not in f.parts:z.write(f,Path('VocaBox')/f.relative_to(root))
with (out/'SHA256SUMS.txt').open('w') as f:
 for path in sorted(out.glob('*.zip')):f.write(hashlib.sha256(path.read_bytes()).hexdigest()+'  '+path.name+'\n')
print(out/'VocaBox-Windows-x64.zip');print(out/'VocaBox-Source.zip')
