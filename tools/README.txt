Tcgutil is a Linux set of TPM tools:
 * tcg_createkey let you create an AES key
 * tcg_seal let you seal a file with a key
 * createStruct.py is a script to build a structure to then seal it

To compile : 
 * apt-get install libtspi-dev
 * make

_________
Example :
_________
 * Use createStruct.py to build string_struct.data
 * Use createStruct.py to build image_struct.data
 * If any, create a key with tcg_createkey

 * Then:
 ** to seal a string structure :
./tcg_seal -k key1.key -z -p : -o string_struct.seal -i string_struct.data 
(0000)-> SRK(WellKnown : 1)
   (0001)-> key1.key(WellKnown : 1)(00000000) 	| child of SRK
symmetric key : xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx
Password of key1.key : xxx

 ** to seal an image structure :
./tcg_seal -k key1.key -z -p : -o image_struct.seal -i image_struct.data 
(0000)-> SRK(WellKnown : 1)
   (0001)-> key1.key(WellKnown : 1)(00000000) 	| child of SRK
symmetric key : xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx
Password of key1.key : xxx

 * That's it ! You can now boot into grub to try unsealing these structures...

 * To be sure there is no problem unsealing these, you can try from Linux using tcg_unseal :
./tcg_unseal -k key1.key -z -o string.out -i string_struct.seal 
(0000)-> SRK(WellKnown : 1)
   (0001)-> key1.key(WellKnown : 1)(00000000) 	| child of SRK
Password of key1.key : cspn

