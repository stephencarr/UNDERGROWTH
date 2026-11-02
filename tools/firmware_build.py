#!/usr/bin/env python3
"""Build and check the pinned host or hardware app. No device access, no flashing."""
import argparse,hashlib,json,os,pathlib,re,shutil,subprocess,sys
ROOT=pathlib.Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--host',action='store_true');p.add_argument('--base',type=pathlib.Path,default=pathlib.Path(os.environ.get('FM1_BASE',ROOT/'.tools/hardware/fm1-base')));p.add_argument('--release',default='0.1.2-hw1');a=p.parse_args()
base=a.base.resolve();lock=json.loads((ROOT/'firmware/dependencies.json').read_text())
def capture(*cmd):return subprocess.check_output(cmd,text=True).strip()
for path,expected in [(base,lock['fm1_base']['commit']),(base/'bsp',lock['bsp_commit'])]:
    if not path.exists():raise SystemExit('Missing dependency. Run tools/prepare_firmware.py --host-only first.')
    if capture('git','-C',str(path),'rev-parse','HEAD')!=expected:raise SystemExit('Dependency revision mismatch: '+str(path))
    if capture('git','-C',str(path),'diff','--name-only','HEAD'):raise SystemExit('Modified dependency sources: '+str(path))
cmd=[sys.executable,str(base/'tools/build.py'),str(ROOT/'firmware')]
if a.host:
    subprocess.run(cmd+['--host'],check=True)
    for script in sorted((ROOT/'tests/hardware').glob('*.txt')):subprocess.run([str(ROOT/'firmware/build/host/undergrowth_host'),str(script),str(ROOT/'build'/script.stem)],check=True)
    raise SystemExit(0)
if not re.fullmatch(r'\d{1,2}\.\d{1,2}\.\d{1,2}-[A-Za-z0-9]+',a.release):raise SystemExit('Use an explicitly experimental version, e.g. 0.1.2-hw1.')
sdk=pathlib.Path(os.environ.get('AC79_SDK',ROOT/'.tools/hardware/sdk'))
for rel,digest in lock['sdk']['files'].items():
    path=sdk/'cpu/wl82/tools'/rel
    if not path.exists() or hashlib.sha256(path.read_bytes()).hexdigest()!=digest:raise SystemExit('Missing or mismatched SDK asset: '+str(path))
tc=pathlib.Path(os.environ.get('JIELI_TOOLCHAIN','/missing'))
if not (tc/'pi32v2/bin/clang').is_file():raise SystemExit('Hardware package BLOCKED: set JIELI_TOOLCHAIN to the verified compiler; no .fwsc is claimed or generated.')
result=subprocess.run(cmd+['--sdk',str(sdk),'--release',a.release],stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
print(result.stdout);(ROOT/'firmware/build/build-report.txt').write_text(result.stdout)
if result.returncode:raise SystemExit(result.returncode)
pkg=ROOT/'firmware/build'/f'undergrowth-{a.release}.fwsc'
subprocess.run(['node',str(ROOT/'tools/check_fwsc.mjs'),str(base),str(pkg)],check=True)
out=ROOT/'build/releases'/a.release;out.mkdir(parents=True,exist_ok=True)
shutil.copy2(pkg,out/pkg.name);shutil.copy2(ROOT/'firmware/build/build-report.txt',out/'build-report.txt')
for name in ['fm1_install.py','fm1ports.py']:shutil.copy2(base/'tools'/name,out/name)
for source,target in [(base/'LICENSE','MPL-2.0.txt'),(base/'assets/fonts/Barlow-OFL.txt','Barlow-OFL.txt'),(sdk/'LICENSE','SDK-Apache-2.0.txt'),(ROOT/'docs/HARDWARE.md','HARDWARE.md'),(ROOT/'LICENSING.md','LICENSING.md')]:shutil.copy2(source,out/target)
source_files=capture('git','-C',str(ROOT),'ls-files','--cached','--others','--exclude-standard').splitlines()
source_hashes={name:hashlib.sha256((ROOT/name).read_bytes()).hexdigest() for name in source_files if (ROOT/name).is_file()}
manifest={'version':a.release,'experimental':True,'hardware_validated':False,'source_commit':capture('git','-C',str(ROOT),'rev-parse','HEAD'),'source_dirty':bool(capture('git','-C',str(ROOT),'status','--porcelain')),'source_sha256':source_hashes,'dependencies':lock,'sha256':hashlib.sha256(pkg.read_bytes()).hexdigest()}
(out/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
(out/'SHA256SUMS').write_text(''.join(hashlib.sha256(f.read_bytes()).hexdigest()+'  '+f.name+'\n' for f in sorted(out.iterdir()) if f.is_file() and f.name!='SHA256SUMS'))
print('EXPERIMENTAL package verified:',out)
