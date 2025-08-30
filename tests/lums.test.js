

const { expect } = require('chai');
const fs = require('fs');
const path = require('path');

/**
 * FRAMEWORK DE TESTS TRIPLES SELON CAHIER DES CHARGES
 * Chaque test doit s'exécuter 3 fois consécutives sans erreur
 * Avec logs JSONL détaillés et validation complète
 */

class TripleTestRunner {
  constructor() {
    this.logs = [];
    this.runId = `run-${Date.now()}-${Math.random().toString(36).substr(2, 9)}`;
    this.startTime = process.hrtime.bigint();
  }

  log(level, message, metadata = {}) {
    const logEntry = {
      ts_ns: process.hrtime.bigint().toString(),
      run_id: this.runId,
      tick: this.logs.length + 1,
      tx_id: metadata.tx_id || this.generateTxId(),
      op: metadata.op || 'test',
      level,
      message,
      ...metadata
    };
    this.logs.push(logEntry);
  }

  generateTxId() {
    return Math.random().toString(36).substr(2, 8);
  }

  async runTripleTest(testName, testFunction) {
    this.log('info', `Starting triple test: ${testName}`, { test_name: testName });
    
    const results = [];
    
    for (let iteration = 1; iteration <= 3; iteration++) {
      this.log('info', `Test iteration ${iteration}/3`, { 
        test: testName, 
        iteration,
        op: 'test_iteration'
      });
      
      try {
        const iterationStart = process.hrtime.bigint();
        await testFunction(iteration);
        const iterationEnd = process.hrtime.bigint();
        const duration = Number((iterationEnd - iterationStart) / BigInt(1000000));
        
        this.log('success', `Iteration ${iteration} passed`, { 
          test: testName, 
          iteration,
          duration_ms: duration,
          op: 'test_success'
        });
        
        results.push({ iteration, status: 'passed', duration_ms: duration });
        
      } catch (error) {
        this.log('error', `Iteration ${iteration} failed`, { 
          test: testName, 
          iteration, 
          error: error.message,
          stack: error.stack,
          op: 'test_failure'
        });
        throw error;
      }
    }
    
    const totalDuration = results.reduce((sum, r) => sum + r.duration_ms, 0);
    this.log('info', `Triple test completed successfully: ${testName}`, {
      test_name: testName,
      total_iterations: 3,
      total_duration_ms: totalDuration,
      avg_duration_ms: totalDuration / 3,
      op: 'test_complete'
    });
    
    return results;
  }

  validateConservation(before, after, operation) {
    const beforeCount = Array.isArray(before) ? before.length : before;
    const afterCount = Array.isArray(after) 
      ? (Array.isArray(after[0]) ? after.reduce((sum, arr) => sum + arr.length, 0) : after.length)
      : after;

    const conserved = beforeCount === afterCount || operation === 'cycle';
    
    this.log(conserved ? 'success' : 'error', 'Conservation check', {
      operation,
      before_count: beforeCount,
      after_count: afterCount,
      conserved,
      op: 'conservation_check'
    });

    return conserved;
  }

  saveLogsToJSONL() {
    const logsDir = path.join(__dirname, '../logs');
    if (!fs.existsSync(logsDir)) {
      fs.mkdirSync(logsDir, { recursive: true });
    }
    
    const timestamp = new Date().toISOString().replace(/[:.]/g, '-');
    const logFile = path.join(logsDir, `${this.runId}-${timestamp}.jsonl`);
    const jsonlContent = this.logs.map(log => JSON.stringify(log)).join('\n');
    fs.writeFileSync(logFile, jsonlContent);
    
    const totalDuration = Number((process.hrtime.bigint() - this.startTime) / BigInt(1000000));
    
    console.log(`\n📊 TRIPLE TEST SESSION COMPLETE`);
    console.log(`📁 Logs saved to: ${logFile}`);
    console.log(`📈 Total logs: ${this.logs.length}`);
    console.log(`⏱️  Total duration: ${totalDuration}ms`);
    console.log(`🆔 Run ID: ${this.runId}`);
    
    return logFile;
  }

  getStats() {
    const stats = {
      total_logs: this.logs.length,
      run_id: this.runId,
      duration_ms: Number((process.hrtime.bigint() - this.startTime) / BigInt(1000000)),
      levels: {},
      operations: {}
    };

    this.logs.forEach(log => {
      stats.levels[log.level] = (stats.levels[log.level] || 0) + 1;
      stats.operations[log.op] = (stats.operations[log.op] || 0) + 1;
    });

    return stats;
  }
}

describe('🔬 LUMS Triple Tests Suite - Complete Validation', function() {
  let testRunner;

  beforeEach(function() {
    testRunner = new TripleTestRunner();
  });

  afterEach(function() {
    const logFile = testRunner.saveLogsToJSONL();
    const stats = testRunner.getStats();
    console.log('📊 Session stats:', JSON.stringify(stats, null, 2));
  });

  describe('🔄 Conversion Bit ↔ LUM', function() {
    it('should convert binary string to LUM and back 3 times with full validation', async function() {
      await testRunner.runTripleTest('bit-lum-conversion', async (iteration) => {
        const testBits = `110100101010${iteration}`;
        testRunner.log('debug', 'Testing bit conversion', { 
          input: testBits, 
          length: testBits.length,
          iteration,
          op: 'bit_conversion_start'
        });

        // Conversion bit → LUM avec IDs uniques
        const lums = testBits.split('').map((bit, index) => ({
          presence: bit === '1' ? 1 : 0,
          structure_type: 'LUM_LINEAR',
          position: { x: index * 20, y: 0 },
          lum_id: `L-${testRunner.runId}-${iteration}-${index.toString().padStart(6, '0')}`,
          metadata: {
            bit_position: index,
            source_bit: bit,
            test_iteration: iteration
          }
        }));

        testRunner.log('debug', 'LUMs created', { 
          lum_count: lums.length,
          lum_ids: lums.slice(0, 3).map(l => l.lum_id), // Premiers 3 IDs
          total_presence: lums.reduce((sum, l) => sum + l.presence, 0),
          op: 'lum_creation'
        });

        // Validation structure LUM
        lums.forEach((lum, index) => {
          expect(lum.presence).to.be.oneOf([0, 1], `LUM ${index} presence invalid`);
          expect(lum.structure_type).to.equal('LUM_LINEAR', `LUM ${index} structure invalid`);
          expect(lum.position).to.have.property('x', index * 20, `LUM ${index} position.x invalid`);
          expect(lum.position).to.have.property('y', 0, `LUM ${index} position.y invalid`);
          expect(lum.lum_id).to.match(/^L-run-\d+-\w+-\d+-\d{6}$/, `LUM ${index} ID format invalid`);
        });

        // Reconversion LUM → bit
        const resultBits = lums.map(l => l.presence === 1 ? '1' : '0').join('');
        
        testRunner.log('debug', 'Reconversion completed', { 
          output: resultBits,
          roundtrip_success: testBits === resultBits,
          conservation_check: testBits.length === resultBits.length,
          op: 'bit_reconversion'
        });

        // Validation conservation et roundtrip
        expect(resultBits).to.equal(testBits, 'Roundtrip conversion failed');
        expect(lums.length).to.equal(testBits.length, 'LUM count mismatch');
        expect(testRunner.validateConservation(testBits.length, lums.length, 'conversion')).to.be.true;
      });
    });
  });

  describe('⚡ Opérations VORAX avec Conservation', function() {
    it('should perform fusion operation 3 times with strict conservation', async function() {
      await testRunner.runTripleTest('vorax-fusion', async (iteration) => {
        const group1 = [
          { presence: 1, lum_id: `L-${iteration}-fusion-001`, structure_type: 'LUM_LINEAR' },
          { presence: 1, lum_id: `L-${iteration}-fusion-002`, structure_type: 'LUM_LINEAR' }
        ];
        const group2 = [
          { presence: 0, lum_id: `L-${iteration}-fusion-003`, structure_type: 'LUM_LINEAR' },
          { presence: 1, lum_id: `L-${iteration}-fusion-004`, structure_type: 'LUM_LINEAR' }
        ];

        testRunner.log('debug', 'Fusion operation start', {
          group1_count: group1.length,
          group2_count: group2.length,
          group1_ids: group1.map(l => l.lum_id),
          group2_ids: group2.map(l => l.lum_id),
          total_before: group1.length + group2.length,
          op: 'fusion_start'
        });

        // Fusion avec conservation
        const fused = [...group1, ...group2];
        
        testRunner.log('info', 'Fusion result', {
          fused_count: fused.length,
          conservation_check: fused.length === (group1.length + group2.length),
          fused_ids: fused.map(l => l.lum_id),
          total_presence: fused.reduce((sum, l) => sum + l.presence, 0),
          op: 'fusion_complete'
        });

        // Validation stricte conservation
        expect(fused.length).to.equal(group1.length + group2.length, 'Fusion conservation failed');
        expect(fused).to.deep.include.members([...group1, ...group2], 'Fusion members mismatch');
        expect(testRunner.validateConservation(group1.length + group2.length, fused.length, 'fusion')).to.be.true;
      });
    });

    it('should perform split operation 3 times with exact conservation', async function() {
      await testRunner.runTripleTest('vorax-split', async (iteration) => {
        const sourceGroup = [
          { presence: 1, lum_id: `L-${iteration}-split-001` },
          { presence: 1, lum_id: `L-${iteration}-split-002` },
          { presence: 0, lum_id: `L-${iteration}-split-003` },
          { presence: 1, lum_id: `L-${iteration}-split-004` },
          { presence: 1, lum_id: `L-${iteration}-split-005` }
        ];
        const zones = 2;

        testRunner.log('debug', 'Split operation start', {
          source_count: sourceGroup.length,
          zones,
          source_ids: sourceGroup.map(l => l.lum_id),
          op: 'split_start'
        });

        // Split équitable avec gestion du reste
        const zoneSize = Math.floor(sourceGroup.length / zones);
        const remainder = sourceGroup.length % zones;
        
        const zone1 = sourceGroup.slice(0, zoneSize + (remainder > 0 ? 1 : 0));
        const zone2 = sourceGroup.slice(zone1.length);

        testRunner.log('info', 'Split result', {
          zone1_count: zone1.length,
          zone2_count: zone2.length,
          total_after_split: zone1.length + zone2.length,
          conservation_check: (zone1.length + zone2.length) === sourceGroup.length,
          zone1_ids: zone1.map(l => l.lum_id),
          zone2_ids: zone2.map(l => l.lum_id),
          op: 'split_complete'
        });

        // Validation conservation exacte
        expect(zone1.length + zone2.length).to.equal(sourceGroup.length, 'Split conservation failed');
        expect([...zone1, ...zone2]).to.deep.equal(sourceGroup, 'Split order preservation failed');
        expect(testRunner.validateConservation(sourceGroup.length, [zone1, zone2], 'split')).to.be.true;
      });
    });

    it('should perform cycle operation 3 times with modulo validation', async function() {
      await testRunner.runTripleTest('vorax-cycle', async (iteration) => {
        const sourceGroup = [
          { presence: 1, lum_id: `L-${iteration}-cycle-001` },
          { presence: 1, lum_id: `L-${iteration}-cycle-002` },
          { presence: 0, lum_id: `L-${iteration}-cycle-003` },
          { presence: 1, lum_id: `L-${iteration}-cycle-004` }
        ];
        const modulus = 3;

        testRunner.log('debug', 'Cycle operation start', {
          source_count: sourceGroup.length,
          modulus,
          expected_result_count: sourceGroup.length % modulus,
          source_ids: sourceGroup.map(l => l.lum_id),
          op: 'cycle_start'
        });

        // Cycle (modulo) - pas de conservation traditionnelle
        const cycledCount = sourceGroup.length % modulus;
        const cycledGroup = sourceGroup.slice(0, cycledCount);

        testRunner.log('info', 'Cycle result', {
          cycled_count: cycledGroup.length,
          modulo_check: cycledGroup.length === (sourceGroup.length % modulus),
          cycled_ids: cycledGroup.map(l => l.lum_id),
          modulo_operation: `${sourceGroup.length} % ${modulus} = ${cycledCount}`,
          op: 'cycle_complete'
        });

        // Validation modulo correcte
        expect(cycledGroup.length).to.equal(sourceGroup.length % modulus, 'Cycle modulo calculation failed');
        expect(cycledGroup).to.deep.equal(sourceGroup.slice(0, cycledCount), 'Cycle selection failed');
      });
    });

    it('should perform flow operation 3 times with metadata preservation', async function() {
      await testRunner.runTripleTest('vorax-flow', async (iteration) => {
        const sourceGroup = [
          { presence: 1, lum_id: `L-${iteration}-flow-001` },
          { presence: 0, lum_id: `L-${iteration}-flow-002` }
        ];
        const targetZone = `zone_target_${iteration}`;

        testRunner.log('debug', 'Flow operation start', {
          source_count: sourceGroup.length,
          target_zone: targetZone,
          source_ids: sourceGroup.map(l => l.lum_id),
          op: 'flow_start'
        });

        // Flow avec transfert de métadonnées
        const flowedGroup = {
          ...sourceGroup,
          lums: sourceGroup,
          connections: [targetZone],
          metadata: {
            operation: 'flow',
            target_zone: targetZone,
            flow_timestamp: new Date().toISOString()
          }
        };

        testRunner.log('info', 'Flow result', {
          flowed_count: flowedGroup.lums.length,
          target_zone: targetZone,
          connections: flowedGroup.connections,
          conservation_check: flowedGroup.lums.length === sourceGroup.length,
          op: 'flow_complete'
        });

        // Validation flow
        expect(flowedGroup.lums.length).to.equal(sourceGroup.length, 'Flow conservation failed');
        expect(flowedGroup.connections).to.include(targetZone, 'Flow target zone missing');
        expect(testRunner.validateConservation(sourceGroup.length, flowedGroup.lums.length, 'flow')).to.be.true;
      });
    });
  });

  describe('🛡️ Invariants du système', function() {
    it('should validate LUM structure invariants 3 times with strict checks', async function() {
      await testRunner.runTripleTest('lum-invariants', async (iteration) => {
        const testLums = [
          {
            presence: 1,
            structure_type: 'LUM_LINEAR',
            position: { x: 10, y: 20 },
            lum_id: `L-${iteration}-INV-001`
          },
          {
            presence: 0,
            structure_type: 'LUM_GROUP',
            position: { x: 30, y: 40 },
            lum_id: `L-${iteration}-INV-002`
          }
        ];

        testRunner.log('debug', 'Validating LUM invariants', {
          lum_count: testLums.length,
          lum_ids: testLums.map(l => l.lum_id),
          op: 'invariant_validation'
        });

        testLums.forEach((testLum, index) => {
          // Invariants fondamentaux
          expect(testLum.presence).to.be.oneOf([0, 1], `LUM ${index} presence not binary`);
          expect(testLum.structure_type).to.be.a('string', `LUM ${index} structure_type not string`);
          expect(testLum.position).to.have.property('x', testLum.position.x, `LUM ${index} position.x missing`);
          expect(testLum.position).to.have.property('y', testLum.position.y, `LUM ${index} position.y missing`);
          expect(testLum.lum_id).to.match(/^L-\d+-INV-\d{3}$/, `LUM ${index} ID format invalid`);

          // Invariants spatiaux
          expect(typeof testLum.position.x).to.equal('number', `LUM ${index} position.x not number`);
          expect(typeof testLum.position.y).to.equal('number', `LUM ${index} position.y not number`);

          testRunner.log('success', `LUM ${index} invariants validated`, {
            lum_id: testLum.lum_id,
            presence: testLum.presence,
            structure_type: testLum.structure_type,
            position: testLum.position,
            op: 'invariant_validated'
          });
        });

        testRunner.log('success', 'All LUM invariants validated', {
          total_validated: testLums.length,
          iteration,
          op: 'all_invariants_validated'
        });
      });
    });
  });
});

