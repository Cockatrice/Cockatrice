import { IndexableType } from 'dexie';
import { App } from '@app/types';

import { dexieService } from '../DexieService';

export class HostDTO extends App.Host {
  save() {
    return dexieService.hosts.put(this);
  }

  static add(host: App.Host): Promise<IndexableType> {
    return dexieService.hosts.add(host);
  }

  static get(id: number): Promise<HostDTO> {
    return dexieService.hosts.where('id').equals(id).first();
  }

  static getAll(): Promise<HostDTO[]> {
    return dexieService.hosts.toArray();
  }

  static bulkAdd(hosts: App.Host[]): Promise<IndexableType> {
    return dexieService.hosts.bulkAdd(hosts);
  }

  static delete(id: number): Promise<void> {
    return dexieService.hosts.delete(id);
  }
}

dexieService.hosts.mapToClass(HostDTO);
