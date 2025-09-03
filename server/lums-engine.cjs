/**
 * LUMS Engine - Node.js Interface
 * Provides JavaScript interface to LUMS C backend
 */

/**
 * Convert bits to LUMS representation
 * @param {string} bitString - Binary string (e.g., "11010")
 * @returns {string} JSON representation of LUMS
 */
function convertBitsToLums(bitString) {
  try {
    // Validate input
    if (!/^[01]+$/.test(bitString)) {
      throw new Error('Invalid bit string. Only 0s and 1s allowed.');
    }

    // Convert each bit to a LUM
    const lums = [];
    for (let i = 0; i < bitString.length; i++) {
      const bit = parseInt(bitString[i]);
      const lum = {
        presence: bit,
        structure_type: bit === 1 ? 'LUM_LINEAR' : 'LUM_EMPTY',
        spatial_data: {
          x: i,
          y: 0
        },
        id: `lum_${i}_${Date.now()}`
      };
      lums.push(lum);
    }

    return JSON.stringify(lums, null, 2);
  } catch (error) {
    throw new Error(`LUMS conversion failed: ${error.message}`);
  }
}

/**
 * Convert LUMS back to bits
 * @param {string} lumsJson - JSON string of LUMS
 * @returns {string} Binary string
 */
function convertLumsToBits(lumsJson) {
  try {
    const lums = JSON.parse(lumsJson);
    let bitString = '';
    
    for (const lum of lums) {
      bitString += lum.presence.toString();
    }
    
    return bitString;
  } catch (error) {
    throw new Error(`LUMS to bits conversion failed: ${error.message}`);
  }
}

/**
 * Perform VORAX fusion operation
 * @param {string} lumsJson1 - First LUM group JSON
 * @param {string} lumsJson2 - Second LUM group JSON
 * @returns {string} Fused LUM group JSON
 */
function fuseLums(lumsJson1, lumsJson2) {
  try {
    const lums1 = JSON.parse(lumsJson1);
    const lums2 = JSON.parse(lumsJson2);
    
    // Simple fusion: combine LUMs and update IDs
    const fusedLums = [...lums1, ...lums2].map((lum, index) => ({
      ...lum,
      id: `fused_lum_${index}_${Date.now()}`,
      structure_type: 'LUM_FUSED'
    }));
    
    return JSON.stringify(fusedLums, null, 2);
  } catch (error) {
    throw new Error(`LUMS fusion failed: ${error.message}`);
  }
}

/**
 * Perform VORAX split operation
 * @param {string} lumsJson - LUM group JSON to split
 * @returns {Array} Array of split LUM groups
 */
function splitLums(lumsJson) {
  try {
    const lums = JSON.parse(lumsJson);
    const midPoint = Math.floor(lums.length / 2);
    
    const group1 = lums.slice(0, midPoint).map((lum, index) => ({
      ...lum,
      id: `split1_lum_${index}_${Date.now()}`,
      structure_type: 'LUM_SPLIT_1'
    }));
    
    const group2 = lums.slice(midPoint).map((lum, index) => ({
      ...lum,
      id: `split2_lum_${index}_${Date.now()}`,
      structure_type: 'LUM_SPLIT_2'
    }));
    
    return [JSON.stringify(group1, null, 2), JSON.stringify(group2, null, 2)];
  } catch (error) {
    throw new Error(`LUMS split failed: ${error.message}`);
  }
}

/**
 * Get engine status
 * @returns {Object} Engine status information
 */
function getStatus() {
  return {
    initialized: true,
    lumCount: 0,
    timestamp: new Date().toISOString(),
    version: '1.0.0'
  };
}

// Export functions directly
module.exports = {
  convertBitsToLums,
  convertLumsToBits,
  fuseLums,
  splitLums,
  getStatus
};