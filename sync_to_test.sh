#!/bin/bash
cd ..
rsync --existing --recursive --verbose ./src/* ../NFC_test/src
rsync --existing --recursive --verbose ./inc/* ../NFC_test/inc
rsync --existing --recursive --verbose ./freertos/* ../NFC_test/freertos

