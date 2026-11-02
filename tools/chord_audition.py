#!/usr/bin/env python3
"""Render original CHORD examples from shared C, without devices or normalisation."""
import argparse,array,ctypes as c,json,math,pathlib,subprocess,wave
root=pathlib.Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--output',type=pathlib.Path,default=root/'work/chord-audition');a=p.parse_args();a.output.mkdir(parents=True,exist_ok=True)
libpath=root/'work/chord-audition.so';libpath.parent.mkdir(exist_ok=True)
subprocess.run(['cc','-std=c99','-shared','-fPIC','-O2','-ffp-contract=off',str(root/'core/undergrowth.c'),'-o',str(libpath)],check=True)
lib=c.CDLL(str(libpath));lib.ug_buffer.restype=c.POINTER(c.c_float);lib.ug_setparam.argtypes=[c.c_int,c.c_int,c.c_float];lib.ug_setfx.argtypes=[c.c_int,c.c_float];lib.ug_chordkey.argtypes=[c.c_int,c.c_int,c.c_float]
settings={'dub-stab':(.2,.35,.2,0,.15,.55,False),'bright-stab':(.9,.85,.4,0,.25,.25,False),'ambient-dry':(.55,.45,1,.7,.8,.15,False),'ambient-room':(.55,.45,1,.7,.8,.15,True)}
metrics={};rows=[];rate=44100
for name,values in settings.items():
 lib.ug_init(rate);tone,colour,decay,attack,release,sweep,room=values
 for param,x in zip([0,1,2,4,5,6],[tone,colour,decay,attack,release,sweep]):lib.ug_setparam(4,param,x)
 lib.ug_setparam(4,8,4/7);lib.ug_setfx(4,0)
 if room:lib.ug_setparam(4,3,1);lib.ug_setfx(0,.8);lib.ug_setfx(1,.6)
 samples=array.array('h');energy=[0.0]*12;peak=0;clipped=0;notes=[48,55,51,58];ambient=name.startswith('ambient');last=48
 for at in range(0,rate*12,147):
  if ambient:
   if at==0:lib.ug_chordkey(48,1,.8)
   if at==rate*4:lib.ug_chordkey(48,0,0)
  elif at<rate*4:
   if at%(rate//2)==0:last=notes[(at//(rate//2))%4];lib.ug_chordkey(last,1,.8)
   if at%(rate//2)==rate*15//100:lib.ug_chordkey(last,0,0)
  lib.ug_render(147);buf=lib.ug_buffer()
  for i in range(294):
   x=buf[i];assert math.isfinite(x);peak=max(peak,abs(x));clipped+=abs(x)>=.94999;energy[(at+i//2)//rate]+=x*x;samples.append(round(x*32767))
 with wave.open(str(a.output/(name+'.wav')),'wb') as w:w.setparams((2,2,rate,0,'NONE','not compressed'));w.writeframes(samples.tobytes())
 metrics[name]={'peak':peak,'clipped_samples':clipped,'rms_per_second':[math.sqrt(e/(rate*2)) for e in energy]};rows.append(f'<p>{name}<br><audio controls src="{name}.wav"></audio></p>')
(a.output/'metrics.json').write_text(json.dumps(metrics,indent=2)+'\n');(a.output/'index.html').write_text('<!doctype html><meta charset="utf-8"><title>CHORD audition</title><h1>CHORD audition</h1><p>Original shared-C synthesis, minor-seventh voicing, no normalisation. Four seconds of played source, then eight seconds of tails. Listening acceptance remains open.</p>'+''.join(rows));print(json.dumps(metrics,indent=2))
