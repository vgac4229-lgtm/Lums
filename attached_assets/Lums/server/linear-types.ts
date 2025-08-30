
```typescript
/**
 * Linear Type System for LUMS/VORAX
 * Ensures resource safety and prevents double-use of LUMs
 */

export enum LinearKind {
  UNIQUE = 'unique',      // Exactly one reference
  SHARED = 'shared',      // Multiple read-only references  
  CONSUMED = 'consumed',  // Resource has been consumed
  BORROWED = 'borrowed'   // Temporary exclusive access
}

export interface LinearType {
  kind: LinearKind;
  resourceId: string;
  borrowedFrom?: string;
  capabilities: Capability[];
  lifetime: Lifetime;
}

export enum Capability {
  READ = 'read',
  WRITE = 'write', 
  SPLIT = 'split',
  MERGE = 'merge',
  TRANSFORM = 'transform',
  DESTRUCT = 'destruct'
}

export interface Lifetime {
  start: number;
  end?: number;
  scope: string;
}

export class LinearTypeChecker {
  private activeResources: Map<string, LinearType> = new Map();
  private borrowStack: Array<BorrowFrame> = [];
  
  checkOperation(op: LumOperation): TypeCheckResult {
    switch (op.type) {
      case 'create':
        return this.checkCreate(op);
      case 'split':
        return this.checkSplit(op);
      case 'merge':
        return this.checkMerge(op);
      case 'consume':
        return this.checkConsume(op);
      default:
        return { valid: false, error: `Unknown operation: ${op.type}` };
    }
  }
  
  private checkCreate(op: LumOperation): TypeCheckResult {
    const newId = op.targets[0];
    
    if (this.activeResources.has(newId)) {
      return { valid: false, error: `Resource ${newId} already exists` };
    }
    
    const linearType: LinearType = {
      kind: LinearKind.UNIQUE,
      resourceId: newId,
      capabilities: [Capability.READ, Capability.WRITE, Capability.SPLIT],
      lifetime: { start: Date.now(), scope: op.scope }
    };
    
    this.activeResources.set(newId, linearType);
    return { valid: true };
  }
  
  private checkSplit(op: LumOperation): TypeCheckResult {
    const sourceId = op.sources[0];
    const source = this.activeResources.get(sourceId);
    
    if (!source) {
      return { valid: false, error: `Source resource ${sourceId} not found` };
    }
    
    if (source.kind === LinearKind.CONSUMED) {
      return { valid: false, error: `Cannot split consumed resource ${sourceId}` };
    }
    
    if (!source.capabilities.includes(Capability.SPLIT)) {
      return { valid: false, error: `Resource ${sourceId} cannot be split` };
    }
    
    // Consume source, create targets
    source.kind = LinearKind.CONSUMED;
    
    for (const targetId of op.targets) {
      const targetType: LinearType = {
        kind: LinearKind.UNIQUE,
        resourceId: targetId,
        capabilities: [...source.capabilities],
        lifetime: { start: Date.now(), scope: op.scope }
      };
      this.activeResources.set(targetId, targetType);
    }
    
    return { valid: true };
  }
  
  private checkMerge(op: LumOperation): TypeCheckResult {
    const sources = op.sources.map(id => this.activeResources.get(id));
    
    // Verify all sources exist and are unique
    for (let i = 0; i < sources.length; i++) {
      const source = sources[i];
      if (!source) {
        return { valid: false, error: `Source resource ${op.sources[i]} not found` };
      }
      if (source.kind !== LinearKind.UNIQUE) {
        return { valid: false, error: `Cannot merge non-unique resource ${op.sources[i]}` };
      }
    }
    
    // Consume all sources
    for (const sourceId of op.sources) {
      const source = this.activeResources.get(sourceId)!;
      source.kind = LinearKind.CONSUMED;
    }
    
    // Create merged target
    const targetId = op.targets[0];
    const mergedCapabilities = this.mergeCapabilities(sources as LinearType[]);
    
    const targetType: LinearType = {
      kind: LinearKind.UNIQUE,
      resourceId: targetId,
      capabilities: mergedCapabilities,
      lifetime: { start: Date.now(), scope: op.scope }
    };
    
    this.activeResources.set(targetId, targetType);
    return { valid: true };
  }
  
  private checkConsume(op: LumOperation): TypeCheckResult {
    const resourceId = op.sources[0];
    const resource = this.activeResources.get(resourceId);
    
    if (!resource) {
      return { valid: false, error: `Resource ${resourceId} not found` };
    }
    
    if (resource.kind === LinearKind.CONSUMED) {
      return { valid: false, error: `Resource ${resourceId} already consumed` };
    }
    
    if (resource.kind === LinearKind.BORROWED) {
      return { valid: false, error: `Cannot consume borrowed resource ${resourceId}` };
    }
    
    resource.kind = LinearKind.CONSUMED;
    return { valid: true };
  }
  
  borrow(resourceId: string, borrower: string): BorrowResult {
    const resource = this.activeResources.get(resourceId);
    
    if (!resource) {
      return { success: false, error: `Resource ${resourceId} not found` };
    }
    
    if (resource.kind !== LinearKind.UNIQUE) {
      return { success: false, error: `Can only borrow unique resources` };
    }
    
    // Create borrow frame
    const borrowFrame: BorrowFrame = {
      resourceId,
      borrower,
      originalKind: resource.kind,
      startTime: Date.now()
    };
    
    resource.kind = LinearKind.BORROWED;
    resource.borrowedFrom = borrower;
    this.borrowStack.push(borrowFrame);
    
    return { success: true, borrowFrame };
  }
  
  returnBorrow(resourceId: string): BorrowResult {
    const frameIndex = this.borrowStack.findIndex(f => f.resourceId === resourceId);
    
    if (frameIndex === -1) {
      return { success: false, error: `No active borrow for ${resourceId}` };
    }
    
    const frame = this.borrowStack[frameIndex];
    const resource = this.activeResources.get(resourceId);
    
    if (!resource) {
      return { success: false, error: `Resource ${resourceId} not found` };
    }
    
    // Restore original kind
    resource.kind = frame.originalKind;
    resource.borrowedFrom = undefined;
    
    // Remove borrow frame
    this.borrowStack.splice(frameIndex, 1);
    
    return { success: true, borrowFrame: frame };
  }
  
  private mergeCapabilities(sources: LinearType[]): Capability[] {
    const allCapabilities = new Set<Capability>();
    
    for (const source of sources) {
      for (const cap of source.capabilities) {
        allCapabilities.add(cap);
      }
    }
    
    return Array.from(allCapabilities);
  }
  
  getResourceState(resourceId: string): LinearType | undefined {
    return this.activeResources.get(resourceId);
  }
  
  listActiveResources(): string[] {
    return Array.from(this.activeResources.keys())
      .filter(id => this.activeResources.get(id)?.kind !== LinearKind.CONSUMED);
  }
}

export interface LumOperation {
  type: 'create' | 'split' | 'merge' | 'consume' | 'transform';
  sources: string[];
  targets: string[];
  scope: string;
  metadata?: any;
}

export interface TypeCheckResult {
  valid: boolean;
  error?: string;
  warnings?: string[];
}

export interface BorrowFrame {
  resourceId: string;
  borrower: string;
  originalKind: LinearKind;
  startTime: number;
}

export interface BorrowResult {
  success: boolean;
  error?: string;
  borrowFrame?: BorrowFrame;
}

// Integration with VIR
export function analyzeLinearTypes(instructions: any[]): LinearType[] {
  const checker = new LinearTypeChecker();
  const types: LinearType[] = [];
  
  for (const instruction of instructions) {
    if (isLumOperation(instruction)) {
      const op = extractLumOperation(instruction);
      const result = checker.checkOperation(op);
      
      if (!result.valid) {
        throw new Error(`Linear type error: ${result.error}`);
      }
      
      // Collect types for each target
      for (const target of op.targets) {
        const type = checker.getResourceState(target);
        if (type) {
          types.push(type);
        }
      }
    }
  }
  
  return types;
}

function isLumOperation(instruction: any): boolean {
  return instruction.opcode?.startsWith('LUM_') || instruction.opcode?.startsWith('VORAX_');
}

function extractLumOperation(instruction: any): LumOperation {
  // Extract operation details from VIR instruction
  return {
    type: instruction.opcode.toLowerCase().replace('lum_', '').replace('vorax_', ''),
    sources: instruction.operands?.filter((op: any) => op.type === 'lum_ref').map((op: any) => op.value) || [],
    targets: instruction.targets || [],
    scope: instruction.metadata?.scope || 'global'
  };
}
```
