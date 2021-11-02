import { IndexableType } from 'dexie';
import { Host } from 'types';

import { dexieService } from '../DexieService';

export class HostDTO extends Host {
  save() {
    return dexieService.hosts.put(this);
  }

  static async add(host: HostDTO): Promise<IndexableType> {
    return dexieService.hosts.add(host);
  }

  static async get(id: number): Promise<HostDTO> {
    return dexieService.hosts.where('id').equals(id).first();
  }

  static async getAll(): Promise<HostDTO[]> {
    return dexieService.hosts.toArray();
  }

  static async bulkAdd(hosts: Host[]): Promise<IndexableType> {
    return dexieService.hosts.bulkAdd(hosts);
  }
};

dexieService.hosts.mapToClass(HostDTO);
