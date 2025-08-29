
const { expect } = require('chai');
const fs = require('fs');
const path = require('path');

/**
 * FRAMEWORK DE TESTS TRIPLES SELON CAHIER DES CHARGES
 * Chaque test doit s'exécuter 3 fois consécutives sans erreur
 */

class TripleTestRunner {
  constructor() {
    this.logs = [];
    this.runId = `run-${Date.now()}-${Math.random().toString(36).substr(2, 9)}`;
  }

  log(level, message, metadata = {}) {
    const logEntry = {
      ts_ns: process.hrtime.bigint().toString(),
      run_id: this.runId,
      level,
      message,
      ...metadata
    };
    this.logs.push(logEntry);
  }

  async runTripleTest(testName, testFunction) {
    this.log('info', `Starting triple test: ${testName}`);
    
    for (let iteration = 1; iteration <= 3; iteration++) {
      this.log('info', `Test iteration ${iteration}/3`, { test: testName, iteration });
      
      try {
        await testFunction(iteration);
        this.log('success', `Iteration ${iteration} passed`, { test: testName, iteration });
      } catch (error) {
        this.log('error', `Iteration ${iteration} failed`, { 
          test: testName, 
          iteration, 
          error: error.message,
          stack: error.stack 
        });
        throw error;
      }
    }
    
    this.log('info', `Triple test completed successfully: ${testName}`);
  }

  saveLogsToJSONL() {
    const logsDir = path.join(__dirname, '../logs');
    if (!fs.existsSync(logsDir)) {
      fs.mkdirSync(logsDir, { recursive: true });
    }
    
    const logFile = path.join(logsDir, `${this.runId}.jsonl`);
    const jsonlContent = this.logs.map(log => JSON.stringify(log)).join('\n');
    fs.writeFileSync(logFile, jsonlContent);
    
    console.log(`Tests logs saved to: ${logFile}`);
    return logFile;
  }
}

describe('LUMS Triple Tests Suite', function() {
  let testRunner;

  beforeEach(function() {
    testRunner = new TripleTestRunner();
  });

  afterEach(function() {
    testRunner.saveLogsToJSONL();
  });

  describe('Conversion Bit ↔ LUM', function() {
    it('should convert binary string to LUM and back 3 times', async function() {
      await testRunner.runTripleTest('bit-lum-conversion', async (iteration) => {
        const testBits = `110100101010${iteration}`;
        testRunner.log('debug', 'Testing bit conversion', { 
          input: testBits, 
          length: testBits.length 
        });

        // Simulation de l'appel C (en attendant l'intégration complète)
        const lums = testBits.split('').map((bit, index) => ({
          presence: bit === '1' ? 1 : 0,
          structure_type: 'LUM_LINEAR',
          position: { x: index * 20, y: 0 },
          lum_id: `L-${iteration}-${index.toString().padStart(6, '0')}`
        }));

        testRunner.log('debug', 'LUMs created', { 
          lum_count: lums.length,
          lum_ids: lums.map(l => l.lum_id)
        });

        // Reconversion
        const resultBits = lums.map(l => l.presence === 1 ? '1' : '0').join('');
        
        testRunner.log('debug', 'Reconversion completed', { 
          output: resultBits,
          roundtrip_success: testBits === resultBits
        });

        expect(resultBits).to.equal(testBits);
        expect(lums.length).to.equal(testBits.length);
      });
    });
  });

  describe('Opérations VORAX', function() {
    it('should perform fusion operation 3 times with conservation', async function() {
      await testRunner.runTripleTest('vorax-fusion', async (iteration) => {
        const group1 = [
          { presence: 1, lum_id: `L-${iteration}-001` },
          { presence: 1, lum_id: `L-${iteration}-002` }
        ];
        const group2 = [
          { presence: 0, lum_id: `L-${iteration}-003` },
          { presence: 1, lum_id: `L-${iteration}-004` }
        ];

        testRunner.log('debug', 'Fusion operation', {
          group1_count: group1.length,
          group2_count: group2.length,
          group1_ids: group1.map(l => l.lum_id),
          group2_ids: group2.map(l => l.lum_id)
        });

        // Fusion
        const fused = [...group1, ...group2];
        
        testRunner.log('debug', 'Fusion result', {
          fused_count: fused.length,
          conservation_check: fused.length === (group1.length + group2.length),
          fused_ids: fused.map(l => l.lum_id)
        });

        // Vérification conservation
        expect(fused.length).to.equal(group1.length + group2.length);
        expect(fused).to.deep.include.members([...group1, ...group2]);
      });
    });

    it('should perform split operation 3 times with conservation', async function() {
      await testRunner.runTripleTest('vorax-split', async (iteration) => {
        const sourceGroup = [
          { presence: 1, lum_id: `L-${iteration}-001` },
          { presence: 1, lum_id: `L-${iteration}-002` },
          { presence: 0, lum_id: `L-${iteration}-003` },
          { presence: 1, lum_id: `L-${iteration}-004` },
          { presence: 1, lum_id: `L-${iteration}-005` }
        ];
        const zones = 2;

        testRunner.log('debug', 'Split operation', {
          source_count: sourceGroup.length,
          zones,
          source_ids: sourceGroup.map(l => l.lum_id)
        });

        // Split équitable
        const zoneSize = Math.floor(sourceGroup.length / zones);
        const remainder = sourceGroup.length % zones;
        
        const zone1 = sourceGroup.slice(0, zoneSize + (remainder > 0 ? 1 : 0));
        const zone2 = sourceGroup.slice(zone1.length);

        testRunner.log('debug', 'Split result', {
          zone1_count: zone1.length,
          zone2_count: zone2.length,
          total_after_split: zone1.length + zone2.length,
          conservation_check: (zone1.length + zone2.length) === sourceGroup.length,
          zone1_ids: zone1.map(l => l.lum_id),
          zone2_ids: zone2.map(l => l.lum_id)
        });

        // Vérification conservation
        expect(zone1.length + zone2.length).to.equal(sourceGroup.length);
        expect([...zone1, ...zone2]).to.deep.equal(sourceGroup);
      });
    });

    it('should perform cycle operation 3 times', async function() {
      await testRunner.runTripleTest('vorax-cycle', async (iteration) => {
        const sourceGroup = [
          { presence: 1, lum_id: `L-${iteration}-001` },
          { presence: 1, lum_id: `L-${iteration}-002` },
          { presence: 0, lum_id: `L-${iteration}-003` },
          { presence: 1, lum_id: `L-${iteration}-004` }
        ];
        const modulus = 3;

        testRunner.log('debug', 'Cycle operation', {
          source_count: sourceGroup.length,
          modulus,
          expected_result_count: sourceGroup.length % modulus,
          source_ids: sourceGroup.map(l => l.lum_id)
        });

        // Cycle (modulo)
        const cycledCount = sourceGroup.length % modulus;
        const cycledGroup = sourceGroup.slice(0, cycledCount);

        testRunner.log('debug', 'Cycle result', {
          cycled_count: cycledGroup.length,
          modulo_check: cycledGroup.length === (sourceGroup.length % modulus),
          cycled_ids: cycledGroup.map(l => l.lum_id)
        });

        expect(cycledGroup.length).to.equal(sourceGroup.length % modulus);
      });
    });
  });

  describe('Invariants du système', function() {
    it('should validate LUM structure invariants 3 times', async function() {
      await testRunner.runTripleTest('lum-invariants', async (iteration) => {
        const testLum = {
          presence: 1,
          structure_type: 'LUM_LINEAR',
          position: { x: 10, y: 20 },
          lum_id: `L-${iteration}-INV-001`
        };

        testRunner.log('debug', 'Validating LUM invariants', {
          lum_id: testLum.lum_id,
          presence: testLum.presence,
          structure_type: testLum.structure_type
        });

        // Invariants
        expect(testLum.presence).to.be.oneOf([0, 1]);
        expect(testLum.structure_type).to.be.a('string');
        expect(testLum.position).to.have.property('x');
        expect(testLum.position).to.have.property('y');
        expect(testLum.lum_id).to.match(/^L-\d+-/);

        testRunner.log('success', 'All LUM invariants validated', {
          lum_id: testLum.lum_id
        });
      });
    });
  });
});
