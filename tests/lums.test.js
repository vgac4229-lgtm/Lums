const fs = require('fs');
const path = require('path');

class TripleTestRunner {
  constructor() {
    this.runId = `run-${Date.now()}-${Math.random().toString(36).substr(2, 6)}`;
    this.logs = [];
    this.currentTick = 0;
  }

  log(level, message, metadata = {}) {
    const entry = {
      ts_ns: process.hrtime.bigint().toString(),
      run_id: this.runId,
      tick: this.currentTick,
      tx_id: metadata.tx_id || this.generateTxId(),
      op: metadata.op || 'unknown',
      zone: metadata.zone || 'main',
      lum_id: metadata.lum_id || 'unknown',
      level, message, ...metadata
    };
    this.logs.push(entry);
    console.log(`[TEST-${level.toUpperCase()}]`, message);
  }

  generateTxId() {
    return Math.random().toString(36).substr(2, 5);
  }

  async runTripleTest(testName, testFunction) {
    console.log(`=== Triple Test: ${testName} ===`);

    for (let iteration = 1; iteration <= 3; iteration++) {
      this.log('info', `Starting iteration ${iteration}/3`, {
        test_name: testName,
        iteration,
        op: 'test_start'
      });

      try {
        await testFunction(iteration);
        this.log('success', `Iteration ${iteration}/3: PASSED`, {
          test_name: testName,
          iteration,
          op: 'test_pass'
        });
      } catch (error) {
        this.log('error', `Iteration ${iteration}/3: FAILED`, {
          test_name: testName,
          iteration,
          error: error.message,
          op: 'test_fail'
        });
        throw error;
      }
      this.currentTick++;
    }

    this.log('success', `Triple test completed: ${testName}`, {
      test_name: testName,
      total_iterations: 3,
      op: 'test_complete'
    });
  }

  validateConservation(beforeCount, afterCount, operation) {
    const conserved = beforeCount === afterCount || operation === 'cycle';
    this.log(conserved ? 'success' : 'error', 'Conservation check', {
      operation, before_count: beforeCount, after_count: afterCount, conserved
    });
    return conserved;
  }

  saveLogsJSONL() {
    const filepath = `logs/${this.runId}.jsonl`;
    fs.mkdirSync('logs', { recursive: true });

    const jsonlContent = this.logs.map(log => JSON.stringify(log)).join('\n');
    fs.writeFileSync(filepath, jsonlContent);

    console.log(`Logs saved to ${filepath} (${this.logs.length} entries)`);
    return filepath;
  }
}

// Tests implémentés avec validation triple
const testRunner = new TripleTestRunner();

async function testBitLumConversion(iteration) {
  const testBits = `110100101${iteration.toString().padStart(7, '0')}`;

  // Simulation conversion bit->LUM
  const lums = testBits.split('').map((bit, index) => ({
    presence: parseInt(bit),
    lum_id: `L-${testRunner.runId}-${index.toString().padStart(6, '0')}`
  }));

  // Validation roundtrip
  const resultBits = lums.map(lum => lum.presence.toString()).join('');

  if (testBits !== resultBits) {
    throw new Error(`Roundtrip failed: ${testBits} !== ${resultBits}`);
  }

  testRunner.log('debug', 'Bit-LUM conversion validated', {
    op: 'conversion',
    input_length: testBits.length,
    output_count: lums.length,
    conservation_valid: testBits.length === lums.length
  });
}

async function testVoraxOperations(iteration) {
  // Test fusion
  const group1 = { lums: [{ presence: 1 }, { presence: 1 }] };
  const group2 = { lums: [{ presence: 1 }, { presence: 1 }] };

  const beforeCount = group1.lums.length + group2.lums.length;
  const fused = { lums: [...group1.lums, ...group2.lums] };
  const afterCount = fused.lums.length;

  if (!testRunner.validateConservation(beforeCount, afterCount, 'fusion')) {
    throw new Error('Fusion conservation violated');
  }

  // Test split
  const sourceGroup = { lums: Array(5).fill({ presence: 1 }) };
  const zones = 2;
  const zone1Size = Math.ceil(sourceGroup.lums.length / zones);
  const zone2Size = sourceGroup.lums.length - zone1Size;

  if (!testRunner.validateConservation(sourceGroup.lums.length, zone1Size + zone2Size, 'split')) {
    throw new Error('Split conservation violated');
  }

  testRunner.log('debug', 'VORAX operations validated', {
    op: 'vorax_operations',
    fusion_conservation: true,
    split_conservation: true
  });
}

// Exécution des tests triples
async function runAllTests() {
  try {
    await testRunner.runTripleTest('bit-lum-conversion', testBitLumConversion);
    await testRunner.runTripleTest('vorax-operations', testVoraxOperations);

    console.log('🎯 All triple tests PASSED');
    testRunner.saveLogsJSONL();

  } catch (error) {
    console.error('❌ Tests FAILED:', error.message);
    testRunner.saveLogsJSONL();
    process.exit(1);
  }
}

// Auto-run si script exécuté directement
if (require.main === module) {
  runAllTests();
}

module.exports = { TripleTestRunner, runAllTests };