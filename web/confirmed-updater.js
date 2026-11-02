// Bind every update stage to a MIDI pair the user has explicitly identified.
import {Updater} from './vendor/fm1-base/fm1ota.js';

export function pairPresent(access, pair) {
  return !pair.invalidated && access.inputs.get(pair.input.id) === pair.input && access.outputs.get(pair.output.id) === pair.output &&
    pair.input.state !== 'disconnected' && pair.output.state !== 'disconnected';
}

export async function verifyPair(access, pair, filter = () => true) {
  if (!pairPresent(access, pair)) throw new Error('The identified MIDI pair disconnected. Identify the FM1 again.');
  const subset = {inputs: new Map([[pair.input.id, pair.input]]), outputs: new Map([[pair.output.id, pair.output]])};
  const lost = () => {if (!pairPresent(access, pair)) pair.invalidated = true;};
  access.addEventListener?.('statechange', lost);
  let found;
  try { found = await new Updater(subset).find(filter); }
  finally { access.removeEventListener?.('statechange', lost); }
  if (!found || !pairPresent(access, pair)) {
    found?.link.close();
    throw new Error('The confirmed MIDI pair did not report the expected FM1 identity. Identify it again.');
  }
  return found;
}

export class ConfirmedUpdater extends Updater {
  constructor(access, pair, identity, log, confirmNextPair) {
    super(access, log);
    this.pair = pair;
    this.identity = identity;
    this.confirmNextPair = confirmNextPair;
  }
  async find(filter) {
    return verifyPair(this.access, this.pair, id => id.text === this.identity && (!filter || filter(id)));
  }
  // Re-enumeration cannot establish physical device identity through Web MIDI.
  // Require another explicit identification/confirmation instead of choosing a different port automatically.
  async waitFor(filter, milliseconds) {
    return this.confirmNextPair(filter, milliseconds);
  }
}
