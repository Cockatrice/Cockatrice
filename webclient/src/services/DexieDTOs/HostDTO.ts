import { IndexableType } from 'dexie';
import { Host } from 'types';

import { dexieService } from '../DexieService';

export class HostDTO extends Host {
  save() {
    return dexieService.hosts.put(this);
  }

  static add(host: Host): Promise<IndexableType> {
    return dexieService.hosts.add(host);
  }

  static get(id: number): Promise<HostDTO> {
    return dexieService.hosts.where('id').equals(id).first();
  }

  static getAll(): Promise<HostDTO[]> {
    return dexieService.hosts.toArray();
  }

  static bulkAdd(hosts: Host[]): Promise<IndexableType> {
    return dexieService.hosts.bulkAdd(hosts);
  }

  static delete(id: string): Promise<void> {
    return dexieService.hosts.delete(id);
  }
};

dexieService.hosts.mapToClass(HostDTO);
