import type { LUM, LUMGroup, VoraxOperation } from "@shared/schema";

export class LumsEngine {
  private zones: Map<string, LUMGroup> = new Map();
  private memory: Map<string, LUMGroup> = new Map();

  // Convert binary string to LUMS
  encodeBitsToLums(bits: string): LUM[] {
    return bits.split('').map((bit, index) => ({
      presence: parseInt(bit),
      structureType: 'linear' as const,
      position: { x: index * 20, y: 0 },
    }));
  }

  // Convert LUMS back to binary
  decodeLumsToBits(lums: LUM[]): string {
    return lums.map(lum => lum.presence.toString()).join('');
  }

  // Create a LUM group
  createGroup(lums: LUM[], groupType: 'linear' | 'cluster' | 'node' | 'memory' = 'linear'): LUMGroup {
    return {
      id: `group-${Date.now()}-${Math.random().toString(36).substr(2, 9)}`,
      lums,
      groupType,
    };
  }

  // VORAX Operations
  
  // Fusion (⧉) - Combine groups
  fusion(group1: LUMGroup, group2: LUMGroup): LUMGroup {
    const combinedLums = [...group1.lums, ...group2.lums];
    return this.createGroup(combinedLums, 'cluster');
  }

  // Split (⇅) - Distribute into zones
  split(group: LUMGroup, zones: number): LUMGroup[] {
    const lumsPerZone = Math.floor(group.lums.length / zones);
    const remainder = group.lums.length % zones;
    
    const result: LUMGroup[] = [];
    let currentIndex = 0;
    
    for (let i = 0; i < zones; i++) {
      const zoneSize = lumsPerZone + (i < remainder ? 1 : 0);
      const zoneLums = group.lums.slice(currentIndex, currentIndex + zoneSize);
      result.push(this.createGroup(zoneLums, 'linear'));
      currentIndex += zoneSize;
    }
    
    return result;
  }

  // Cycle (⟲) - Modular reduction
  cycle(group: LUMGroup, modulo: number): LUMGroup {
    const resultLums = group.lums.slice(0, group.lums.length % modulo);
    return this.createGroup(resultLums, 'node');
  }

  // Flow (→) - Transfer between zones
  flow(sourceGroup: LUMGroup, targetZone: string): LUMGroup {
    // Create a copy with target zone reference
    return {
      ...sourceGroup,
      id: `flow-${Date.now()}`,
      connections: [targetZone],
    };
  }

  // Zone management
  setZone(name: string, group: LUMGroup): void {
    this.zones.set(name, group);
  }

  getZone(name: string): LUMGroup | undefined {
    return this.zones.get(name);
  }

  getAllZones(): Record<string, LUMGroup> {
    return Object.fromEntries(this.zones.entries());
  }

  // Memory management
  storeInMemory(name: string, group: LUMGroup): void {
    this.memory.set(name, group);
  }

  retrieveFromMemory(name: string): LUMGroup | undefined {
    return this.memory.get(name);
  }

  // Execute VORAX operation
  executeOperation(operation: VoraxOperation, groups: LUMGroup[]): LUMGroup | LUMGroup[] {
    switch (operation.name) {
      case 'fusion':
        if (groups.length >= 2) {
          return this.fusion(groups[0], groups[1]);
        }
        throw new Error('Fusion requires at least 2 groups');
        
      case 'split':
        if (groups.length >= 1) {
          const zones = operation.parameters?.zones || 2;
          return this.split(groups[0], zones);
        }
        throw new Error('Split requires at least 1 group');
        
      case 'cycle':
        if (groups.length >= 1) {
          const modulo = operation.parameters?.modulo || 3;
          return this.cycle(groups[0], modulo);
        }
        throw new Error('Cycle requires at least 1 group');
        
      case 'flow':
        if (groups.length >= 1) {
          const targetZone = operation.parameters?.targetZone || 'default';
          return this.flow(groups[0], targetZone);
        }
        throw new Error('Flow requires at least 1 group');
        
      default:
        throw new Error(`Unknown operation: ${operation.name}`);
    }
  }

  // Validation complète
  validateLum(lum: LUM): boolean {
    // Validation stricte de la présence
    if (typeof lum.presence !== 'number' || (lum.presence !== 0 && lum.presence !== 1)) {
      return false;
    }
    
    // Validation du type de structure
    if (!['linear', 'group', 'node', 'cycle'].includes(lum.structureType)) {
      return false;
    }
    
    // Validation de la position
    if (!lum.position || 
        typeof lum.position.x !== 'number' || 
        typeof lum.position.y !== 'number' ||
        lum.position.x < -10000 || lum.position.x > 10000 ||
        lum.position.y < -10000 || lum.position.y > 10000) {
      return false;
    }
    
    return true;
  }

  validateGroup(group: LUMGroup): boolean {
    return (
      typeof group.id === 'string' &&
      Array.isArray(group.lums) &&
      group.lums.every(lum => this.validateLum(lum)) &&
      ['linear', 'cluster', 'node', 'memory'].includes(group.groupType)
    );
  }
}

// Singleton instance
export const lumsEngine = new LumsEngine();
