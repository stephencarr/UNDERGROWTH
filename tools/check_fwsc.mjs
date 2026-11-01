// Validate a real package using the pinned platform's format parser; never contacts hardware.
import fs from 'node:fs';import assert from 'node:assert/strict';
const [base,path]=process.argv.slice(2);const source=fs.readFileSync(base+'/web/fm1pkg.js','utf8');
const {productOf,logicalImage,checkUfw,ufwEntries}=await import('data:text/javascript;base64,'+Buffer.from(source).toString('base64'));
const raw=new Uint8Array(fs.readFileSync(path)),product=productOf(raw);assert(/^FM-1_731\d{6}$/.test(product));
const logical=logicalImage(raw);checkUfw(logical);const parts=ufwEntries(logical);assert(parts[0]&&parts[100]);assert.equal(parts[0].len,0x93000);
const loader=logical.slice(parts[100].off,parts[100].off+parts[100].len);assert(Buffer.from(loader).includes(Buffer.from('FELUCCA-LOADER-1')));
const corrupt=logical.slice();corrupt[parts[100].off+parts[100].len-1]^=1;assert.throws(()=>checkUfw(corrupt));
console.log('PASS package identity, section sizes, loader marker, all UFW CRCs and corrupt-package rejection:',product);
