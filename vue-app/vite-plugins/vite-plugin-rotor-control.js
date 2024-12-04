/*
  Plugin that reads the gzipped output file of the rotor control vue app and 
  inserts it into the firmware source code file as a byte array.
*/

import fs from 'fs'
import { version } from "../package.json"

function rotorControl(config = {appPath: '', sourcePath: ''}) {
  let data = [];
  
  function getByteArray(filePath) {
    let fileData = fs.readFileSync(filePath).toString('hex');
    for (var i = 0; i < fileData.length; i += 2)
      data.push('0x' + fileData[i] + '' + fileData[i + 1]);
  }

  function createSourceFile(sourcePath) {
    let sourceContent = ''
    sourceContent += '#ifndef APPINDEX_H\n';
    sourceContent += '#define APPINDEX_H\n\n';
    sourceContent += '#include <Arduino.h>\n\n';
    sourceContent += `#define UI_VERSION "${version}"\n\n`;
    sourceContent += `#define index_html_gzip_len ${data.length}\n`;
    sourceContent += `const uint8_t index_html_gzip[] PROGMEM = {${data.join(', ')}};\n\n`;
    sourceContent += '#endif //APPINDEX_H\n'

    fs.writeFile(sourcePath, sourceContent, function (err) {
      if (err) throw err;
      console.log(`AppIndex header file created (Size of data: ${data.length})`);
    }); 
  }

  return {
    name: 'rotor-control',
    apply: 'build',

    closeBundle() {
      getByteArray(config.appPath);
      createSourceFile(config.sourcePath);
    }
  };
}

export default rotorControl;
