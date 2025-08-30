
// Linear Type System - Phase Moyen Terme
export interface LUMOwnership {
  lumId: string;
  state: 'owned' | 'borrowed' | 'moved' | 'consumed';
  lifetimeStart: number;
  lifetimeEnd?: number;
  borrowedBy?: string;
}

export class LinearTypeChecker {
  private ownerships: Map<string, LUMOwnership> = new Map();
  private currentTick: number = 0;

  checkLinearConstraints(
    operation: string,
    inputLums: string[],
    outputLums: string[]
  ): LinearConstraintResult {
    const violations: string[] = [];

    // Vérifier que tous les LUMs d'entrée sont disponibles
    for (const lumId of inputLums) {
      const ownership = this.ownerships.get(lumId);
      if (!ownership) {
        violations.push(`LUM ${lumId} not found in ownership table`);
        continue;
      }

      if (ownership.state === 'moved' || ownership.state === 'consumed') {
        violations.push(`LUM ${lumId} already consumed, cannot use again`);
      }

      if (ownership.state === 'borrowed' && ownership.borrowedBy !== operation) {
        violations.push(`LUM ${lumId} borrowed by ${ownership.borrowedBy}, cannot use`);
      }
    }

    // Pour les opérations conservatives, vérifier la conservation
    if (['fusion', 'split', 'flow'].includes(operation)) {
      if (operation === 'fusion' && outputLums.length !== inputLums.length) {
        violations.push(`Fusion must conserve LUM count: ${inputLums.length} → ${outputLums.length}`);
      }
    }

    // Marquer les LUMs d'entrée comme consommés
    if (violations.length === 0) {
      for (const lumId of inputLums) {
        this.consumeLUM(lumId, operation);
      }

      // Créer ownership pour les nouveaux LUMs
      for (const lumId of outputLums) {
        this.createLUM(lumId, operation);
      }
    }

    return {
      valid: violations.length === 0,
      violations,
      inputLums,
      outputLums,
      operation,
      tick: this.currentTick++
    };
  }

  private consumeLUM(lumId: string, operation: string): void {
    const ownership = this.ownerships.get(lumId);
    if (ownership) {
      ownership.state = 'consumed';
      ownership.lifetimeEnd = this.currentTick;
      
      logger.log('debug', 'LUM consumed by linear type system', {
        op: 'linear_consume',
        lum_id: lumId,
        operation,
        tick: this.currentTick
      });
    }
  }

  private createLUM(lumId: string, operation: string): void {
    this.ownerships.set(lumId, {
      lumId,
      state: 'owned',
      lifetimeStart: this.currentTick,
      borrowedBy: undefined
    });

    logger.log('debug', 'LUM created by linear type system', {
      op: 'linear_create',
      lum_id: lumId,
      operation,
      tick: this.currentTick
    });
  }

  borrowLUM(lumId: string, borrower: string): boolean {
    const ownership = this.ownerships.get(lumId);
    if (!ownership || ownership.state !== 'owned') {
      return false;
    }

    ownership.state = 'borrowed';
    ownership.borrowedBy = borrower;
    return true;
  }

  returnLUM(lumId: string): boolean {
    const ownership = this.ownerships.get(lumId);
    if (!ownership || ownership.state !== 'borrowed') {
      return false;
    }

    ownership.state = 'owned';
    ownership.borrowedBy = undefined;
    return true;
  }

  getOwnershipStats(): OwnershipStats {
    const stats = {
      total_lums: this.ownerships.size,
      owned: 0,
      borrowed: 0,
      moved: 0,
      consumed: 0,
      current_tick: this.currentTick
    };

    for (const ownership of this.ownerships.values()) {
      stats[ownership.state]++;
    }

    return stats;
  }
}

interface LinearConstraintResult {
  valid: boolean;
  violations: string[];
  inputLums: string[];
  outputLums: string[];
  operation: string;
  tick: number;
}

interface OwnershipStats {
  total_lums: number;
  owned: number;
  borrowed: number;
  moved: number;
  consumed: number;
  current_tick: number;
}

// Instance globale pour le système de types linéaires
export const linearTypeChecker = new LinearTypeChecker();
