/*
  Vite plugin that reads the gzipped output file of the rotor control vue app and 
  inserts it into the C++ firmware source code file as a byte array inside a header file.
*/

import { readFile, writeFile, copyFile } from 'node:fs/promises'
import path from 'path'
import { version } from "../package.json"

function rotorControl(config = {source: '', filename: '', destination: ''}) {
  let data = [];
  let outDir;
  
  // Load in source file as array of hex binary data.
  async function getByteArray() {
    try {
      let fileData = await readFile(config.source);
      fileData = fileData.toString('hex');
      for (var i = 0; i < fileData.length; i += 2) {
        data.push(`0x${fileData[i]}${fileData[i + 1]}`);
      }
    } catch (err) {
      throw new Error(`Failed to read ${config.source}`, {cause: err});
    }
  }

  // Create C++ header file content from loaded source
  function getSourceCode() {
    let sourceCode = '';
    sourceCode += '#ifndef APPINDEX_H\n';
    sourceCode += '#define APPINDEX_H\n\n';
    sourceCode += '#include <Arduino.h>\n\n';
    sourceCode += `#define UI_VERSION "${version}"\n\n`;
    sourceCode += `#define index_html_gzip_len ${data.length}\n`;
    sourceCode += `const uint8_t index_html_gzip[] PROGMEM = {${data.join(', ')}};\n\n`;
    sourceCode += '#endif //APPINDEX_H\n';
    return sourceCode;
  }

  // Create C++ header file from source using the specified filename
  async function createSourceFile() {
    try {
      await writeFile(path.join(outDir, config.filename), getSourceCode());
      console.log(`[Rotor-Control] ${config.filename} header file created in ${outDir} (${data.length} B)`);
    } catch (err) {
      throw new Error (`Failed to create file ${config.filename} in ${outDir}`, {cause: err});
    }
  }

  // Copy created header file to specified destination
  async function copySourceFile() {
    const src = path.join(outDir, config.filename);
    const dest = path.join(config.destination, config.filename);
    try {
      await copyFile(src, dest);
      console.log(`[Rotor-Control] Copied ${src} to ${dest}`);
    } catch (err) {
      throw new Error(`Failed to copy from ${src} to ${dest}`, {cause: err});
    }
  }

  // Run plugin
  async function run() {
    try {
      await getByteArray();
      await createSourceFile();
      await copySourceFile();
    } catch (err) {
      console.error(`[Rotor-Control] ${err} | Cause: ${err.cause}`);
    }
  }

  /* ==================== */

  return {
    name: 'rotor-control',
    apply: 'build',

    configResolved(viteConfig) {
      outDir = viteConfig.build.outDir;
    },

    closeBundle() {
      run();
    }
  };
}

export default rotorControl;
