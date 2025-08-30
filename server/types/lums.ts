// TypeScript interfaces that match the C structures
export interface LUM {
  presence: number; // 0 or 1
  structureType: 'linear' | 'group' | 'node' | 'cycle';
  spatialData?: any;
  position: {
    x: number;
    y: number;
  };
}

export interface LUMGroup {
  id: string;
  lums: LUM[];
  count: number;
  groupType: 'linear' | 'cluster' | 'node' | 'memory';
  connections?: LUMGroup[];
  spatialData?: any;
}

export interface VoraxZone {
  name: string;
  group?: LUMGroup;
  bounds: {
    x: number;
    y: number;
    width: number;
    height: number;
  };
}

export interface VoraxMemory {
  name: string;
  storedGroup?: LUMGroup;
  timestamp: number;
}

export interface VoraxEngineC {
  zones: VoraxZone[];
  zoneCount: number;
  memorySlots: VoraxMemory[];
  memoryCount: number;
  lastError?: string;
}

// Operation result types
export interface OperationResult {
  success: boolean;
  error?: string;
  data?: any;
}

export interface ConservationCheck {
  beforeCount: number;
  afterCount: number;
  conserved: boolean;
  operation: string;
}
