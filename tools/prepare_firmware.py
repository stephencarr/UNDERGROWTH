#!/usr/bin/env python3
"""Fetch pinned build dependencies. Never connects to an FM1 or flashes hardware."""
import argparse, hashlib, json, pathlib, subprocess, tarfile, urllib.request
ROOT=pathlib.Path(__file__).resolve().parents[1]
lock=json.loads((ROOT/'firmware/dependencies.json').read_text())
p=argparse.ArgumentParser();p.add_argument('--host-only',action='store_true');p.add_argument('--destination',type=pathlib.Path,default=ROOT/'.tools/hardware');args=p.parse_args();dest=args.destination.resolve();dest.mkdir(parents=True,exist_ok=True)
def run(*cmd):subprocess.run(cmd,check=True)
def fetch(url):
    with urllib.request.urlopen(url,timeout=120) as r:return r.read()
base=dest/'fm1-base'
created=not base.exists()
if created:run('git','clone','--no-checkout',lock['fm1_base']['url'],str(base))
if not created and subprocess.check_output(['git','-C',str(base),'status','--porcelain']).strip():raise SystemExit('Dependency checkout has changes; refusing to overwrite it.')
run('git','-C',str(base),'checkout','--detach',lock['fm1_base']['commit']);run('git','-C',str(base),'submodule','update','--init','--recursive')
print('FM1_BASE='+str(base),flush=True)
if args.host_only:raise SystemExit(0)
sdk=dest/'sdk';spec=lock['sdk'];prefix='https://raw.githubusercontent.com/'+spec['url'].removeprefix('https://github.com/')+'/'+spec['commit']+'/'
for rel,digest in spec['files'].items():
    path=sdk/'cpu/wl82/tools'/rel
    data=path.read_bytes() if path.exists() and hashlib.sha256(path.read_bytes()).hexdigest()==digest else fetch(prefix+'cpu/wl82/tools/'+rel)
    if hashlib.sha256(data).hexdigest()!=digest:raise SystemExit('SDK checksum mismatch: '+rel)
    path.parent.mkdir(parents=True,exist_ok=True);path.write_bytes(data)
(sdk/'LICENSE').write_bytes(fetch(prefix+'LICENSE'))
print('AC79_SDK='+str(sdk),flush=True)
archive=dest/'jieli-toolchain.tar.xz';spec=lock['toolchain']
if not archive.exists():
    temp=archive.with_suffix('.part')
    try:
        with urllib.request.urlopen(spec['url'],timeout=120) as r,temp.open('wb') as out:
            while chunk:=r.read(1024*1024):out.write(chunk)
        temp.replace(archive)
    except Exception:
        temp.unlink(missing_ok=True);raise
if hashlib.sha256(archive.read_bytes()).hexdigest()!=spec['sha256']:raise SystemExit('Toolchain archive checksum mismatch; nothing extracted.')
target=dest/'toolchain';target.mkdir(exist_ok=True)
with tarfile.open(archive) as tar:tar.extractall(target,filter='data')
candidates=list(target.glob('**/pi32v2/bin/clang'))
if len(candidates)!=1:raise SystemExit('Unexpected compiler archive layout.')
print('JIELI_TOOLCHAIN='+str(candidates[0].parents[2]),flush=True)
