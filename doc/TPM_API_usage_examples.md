### Playing with the TPM tools

```
  $ ./tcg_readpcr
  PCR[00] : 838681fbf2e1d9de2372cdeb371dcebf43cb72d5
  PCR[01] : 3a3f780f11a4b49969fcaa80cd6e3957c33b2275
  PCR[02] : 5385babe9a148fca7c71ff29e30dcde38e25acd5
  PCR[03] : 3a3f780f11a4b49969fcaa80cd6e3957c33b2275
  PCR[04] : a8bbdac0edf78fefe73062bd7b81e03b59fede06
  PCR[05] : 8ec59d7bb4b31050958fe3f73b77978bc69fd590
  PCR[06] : 3a3f780f11a4b49969fcaa80cd6e3957c33b2275
  PCR[07] : 3a3f780f11a4b49969fcaa80cd6e3957c33b2275
  PCR[08] : eae9f37855e7d58033a6830dd51c33779674bf4a
  PCR[09] : 660fb96ef987e36ad0f242544c1db68e09889b8a
  PCR[10] : 727b152b38bc0ded60cd9909fdcc750135eb4b8d
  PCR[11] : 0000000000000000000000000000000000000000
  PCR[12] : 0000000000000000000000000000000000000000
  PCR[13] : 0000000000000000000000000000000000000000
  PCR[14] : 0000000000000000000000000000000000000000
  PCR[15] : 0000000000000000000000000000000000000000
  PCR[16] : 0000000000000000000000000000000000000000
  PCR[17] : ffffffffffffffffffffffffffffffffffffffff
  PCR[18] : ffffffffffffffffffffffffffffffffffffffff
  PCR[19] : ffffffffffffffffffffffffffffffffffffffff
  PCR[20] : ffffffffffffffffffffffffffffffffffffffff
  PCR[21] : ffffffffffffffffffffffffffffffffffffffff
  PCR[22] : ffffffffffffffffffffffffffffffffffffffff
  PCR[23] : 0000000000000000000000000000000000000000

  $ ./tcg_extend -p 12 -o /bin/ls

  $ ./tcg_readpcr
  PCR[00] : 838681fbf2e1d9de2372cdeb371dcebf43cb72d5
  PCR[01] : 3a3f780f11a4b49969fcaa80cd6e3957c33b2275
  PCR[02] : 5385babe9a148fca7c71ff29e30dcde38e25acd5
  PCR[03] : 3a3f780f11a4b49969fcaa80cd6e3957c33b2275
  PCR[04] : a8bbdac0edf78fefe73062bd7b81e03b59fede06
  PCR[05] : 8ec59d7bb4b31050958fe3f73b77978bc69fd590
  PCR[06] : 3a3f780f11a4b49969fcaa80cd6e3957c33b2275
  PCR[07] : 3a3f780f11a4b49969fcaa80cd6e3957c33b2275
  PCR[08] : eae9f37855e7d58033a6830dd51c33779674bf4a
  PCR[09] : 660fb96ef987e36ad0f242544c1db68e09889b8a
  PCR[10] : 727b152b38bc0ded60cd9909fdcc750135eb4b8d
  PCR[11] : 0000000000000000000000000000000000000000
  PCR[12] : b80de5d138758541c5f05265ad144ab9fa86d1db
  PCR[13] : 0000000000000000000000000000000000000000
  PCR[14] : 0000000000000000000000000000000000000000
  PCR[15] : 0000000000000000000000000000000000000000
  PCR[16] : 0000000000000000000000000000000000000000
  PCR[17] : ffffffffffffffffffffffffffffffffffffffff
  PCR[18] : ffffffffffffffffffffffffffffffffffffffff
  PCR[19] : ffffffffffffffffffffffffffffffffffffffff
  PCR[20] : ffffffffffffffffffffffffffffffffffffffff
  PCR[21] : ffffffffffffffffffffffffffffffffffffffff
  PCR[22] : ffffffffffffffffffffffffffffffffffffffff
  PCR[23] : 0000000000000000000000000000000000000000

  $ ./tcg_createkey -z 1 key.data 12
  (0000)-> SRK(WellKnown : 1)
  (0001)-> key.data(WellKnown : 1)(00001000)
  | child of SRK

  $ ls -lh key.data
  -rw-r--r-- 1 fgy fgy 603 avril 4 10:44 key.data
  echo "data" > clear.txt

  $ cat clear.txt
  data

  $ ./tcg_seal -k key.data -z -i clear.txt -o cipher.txt
  (0000)-> SRK(WellKnown : 1)
  (0001)-> key.data(WellKnown : 1)(00000000)
  | child of SRK
  symmetric key : f5 90 47 8c b7 e5 5d bd bf 3b 57 70 3f 73 56 b5 4b a1 cc 00 c4 bd bd 03 8...

  $ hexdump -C cipher.txt
  00000000 0c 01 00 00 01 01 00 00 00 00 00 00 00 00 01 00 |................|
  00000010 64 4f 89 42 f2 69 3d e7 71 b7 93 fa 96 a0 e5 aa |dO.B.i=.q.......|
  00000020 ee 6e 81 71 1e f7 95 a7 74 df ca 80 cc a1 62 25 |.n.q....t.....b%|
  00000030 00 a1 4c 0d 60 c4 01 33 d0 a4 3c 20 fa b1 59 8d |..L.`..3... ..Y.|
  00000040 85 36 9a 1f 11 b5 b5 dd bc 29 2d ac a6 ed 8c ea |.6.......)-.....|
  00000050 b5 ee 18 6a 15 9c 74 34 d2 c6 1d 26 60 86 67 fd |...j..t4....`.g.|
  00000060 9d f1 59 67 c5 8c 0f ab 67 2a 7a ee 79 a0 cf 00 |..Yg....g*z.y...|
  00000070 22 5c dd f6 99 ba 7a a3 a2 28 22 84 3a 2c 10 1e |"\....z..(".:,..|
  00000080 06 95 41 eb 68 56 6f 0b 1e ae 65 c5 2e c1 27 a4 |..A.hVo...e...'.|
  00000090 09 7f 48 c3 aa 48 73 e6 67 c9 48 38 5e 8e 25 f4 |..H..Hs.g.H8^.%.|
  000000a0 27 f5 ba 60 87 93 b5 ad dc c3 08 4f 2e 8a d2 53 |'..`.......O...S|
  000000b0 d6 16 d5 24 fa 0d 29 6d 6a 05 3e a0 00 75 b1 1f |...$..)mj.>..u..|
  000000c0 6c a9 2b 19 c4 f1 29 e7 2a 6b e1 c1 64 b6 cf 6a |l.+...).*k..d..j|
  000000d0 91 c9 52 e7 33 18 ec 30 9e 7f af 80 4d 85 02 ad |..R.3..0....M...|
  000000e0 95 0b 0a 57 ba 97 99 ae 43 18 59 f1 54 70 67 68 |...W....C.Y.Tpgh|
  000000f0 ed 8e ab 9e 89 80 6d 72 80 5a 61 c9 31 28 a1 a7 |......mr.Za.1(..|
  00000100 b6 23 54 c9 0a 1d 14 61 8e fe 91 8c 4f e4 03 df |.#T....a....O...|
  00000110 29 17 de 09 d0 44 ab fa 0d 7d c6 c9 b9 1b 61 b4 |)....D...}....a.|
  00000120

 $ ./tcg_unseal -k key.data -z -i cipher.txt -o decipher.txt
 (0000)-> SRK(WellKnown : 1)
 (0001)-> key.data(WellKnown : 1)(00000000)
 | child of SRK

  $ cat decipher.txt
  data

 $ ./tcg_testaes
 aes_encrypt_ecb OK
 aes_decrypt_ecb OK
```