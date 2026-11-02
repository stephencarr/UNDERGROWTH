#!/usr/bin/env python3
"""Render repeatable, unnormalised ROOT/empty-SEQ ENV comparisons. No device IO.
Example: python3 tools/reverb_audition.py --output work/audition
Optional --reference is a precompiled previous-core .so for A/B comparison.
"""
import argparse, array, ctypes as c, html, json, math, pathlib, subprocess, wave
ROOT=pathlib.Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--output',type=pathlib.Path,required=True);p.add_argument('--reference',type=pathlib.Path);a=p.parse_args()
a.output.mkdir(parents=True,exist_ok=True)
libpath=(ROOT/'work/audition-core.so');libpath.parent.mkdir(exist_ok=True)
subprocess.run(['cc','-shared','-fPIC','-std=c99','-O2','-ffp-contract=off',str(ROOT/'core/undergrowth.c'),'-o',str(libpath)],check=True)
def library(path):
 lib=c.CDLL(str(path.resolve()));lib.ug_buffer.restype=c.POINTER(c.c_float)
 lib.ug_setparam.argtypes=[c.c_int,c.c_int,c.c_float];lib.ug_setfx.argtypes=[c.c_int,c.c_float];lib.ug_effective.argtypes=[c.c_int,c.c_int];lib.ug_effective.restype=c.c_float
 return lib
libs={'clouds':library(libpath)}
if a.reference:libs['previous']=library(a.reference)
metrics={};rows=[];rate=44100
for layer in ['ROOT','ENV']:
 for model,lib in libs.items():
  for decay in [None,.25,.55,1.0]:
   if decay is None and model!='clouds':continue
   wet=decay is not None;label='dry' if not wet else f'decay-{round(decay*100)}'
   name=f'{layer.lower()}-{model}-{label}.wav';lib.ug_init(rate);lib.ug_setfx(4,0);lib.ug_setfx(0,int(wet));lib.ug_setfx(1,.55 if decay is None else decay)
   lib.ug_setparam(2,16 if layer=='ENV' else 3,1)
   if layer=='ENV':lib.ug_setparam(2,17,1);lib.ug_play(1)
   samples=array.array('h');energy=[0.0]*12;peak=0;clipped=0
   for at in range(0,rate*12,147):
    # Four seconds of source followed by eight seconds of decay. No steps.
    if layer=='ROOT' and at<rate*4:
     if at%(rate//2)==0:lib.ug_key([36,43,39,46][(at//(rate//2))%4],1)
     if at%(rate//2)==rate//5:lib.ug_key([36,43,39,46][(at//(rate//2))%4],0)
    if at==rate*4:
     if layer=='ENV':lib.ug_play(0)
     else:lib.ug_key(46,0)
    assert lib.ug_effective(2,16 if layer=='ENV' else 3)>.999
    lib.ug_render(147);buf=lib.ug_buffer()
    for i in range(294):
     x=buf[i];assert math.isfinite(x);peak=max(peak,abs(x));clipped+=abs(x)>=.94999
     energy[(at+i//2)//rate]+=x*x;samples.append(round(x*32767))
   with wave.open(str(a.output/name),'wb') as w:w.setparams((2,2,rate,0,'NONE','not compressed'));w.writeframes(samples.tobytes())
   metrics[name]={'peak':peak,'clipped_samples':clipped,'rms_per_second':[math.sqrt(e/(rate*2)) for e in energy]}
   rows.append(f'<tr><td>{layer}</td><td>{model}</td><td>{label}</td><td><audio controls preload="none" src="{html.escape(name)}"></audio></td></tr>')
(a.output/'metrics.json').write_text(json.dumps(metrics,indent=2)+'\n')
(a.output/'index.html').write_text('<!doctype html><meta charset="utf-8"><title>UNDERGROWTH reverb comparison</title><style>body{font:17px system-ui;max-width:950px;margin:40px auto;background:#f6f3e9;color:#23372c}td{padding:10px}audio{width:360px}</style><h1>Reverb comparison</h1><p>Same source and master level, no normalisation. Full send and Reverb; Delay return zero. Source stops at four seconds; eight seconds of tail follow. ROOT plays live notes; ENV runs with an empty sequence. “Previous” is 0.4.2. Headphone listening is the acceptance test, not these measurements.</p><table>'+''.join(rows)+'</table>')
print(json.dumps(metrics,indent=2))
