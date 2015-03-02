====================
OpenDTeX Secure Boot
====================

Project Description
===================

The OpenDTeX research project aims at providing trusted building
blocks to ensure strong security properties during the boot chain and
to allow secure execution of isolated enclaves on x86
architectures. OpenDTeX has been achieved with the help of the French
“RAPID” grant process, which targets both civil and defense use cases,
through a consortium composed of AMOSSYS, Bertin Technologies and
Telecom ParisTech.

This project leverages TCG technologies, such as TPM and DRTM, to
provide trusted execution of a minimal TCB (Trusted Computing
Base). Besides, each building block can display proof of integrity up
to the platform user, by implementing the concept of trusted banner,
thus creating a trusted path between the user and the TCB.

The results of this project have been integrated in a Linux-based
prototype, as well as in the PolyXene multi-level security operating
system.

We provide here the implementation of the Secure Boot component in
DRTM mode.

Authors
=======

See the top distribution file ``AUTHORS.txt`` for the detailed and updated list
of authors.

License
=======

This software is licensed under the Modified BSD License. See the
``COPYING.txt`` file for the full license text.

More Information
================

:Website: `https://github.com/AMOSSYS/OpenDTeX-Secure-Boot-DRTM <https://github.com/AMOSSYS/OpenDTeX-Secure-Boot-DRTM>`_
:Email: `etudes@amossys.fr <etudes@amossys.fr>`_
:Twitter: Follow AMOSSYS's official accounts (@amossys)

OpenDTeX developments
=====================

OpenDTeX work notably include:
* A TPM 1.2 API library independent from the BIOS or OS
* A minimal TSS API library independent from the OS
* An extension of Grub 2 (i.e. an SRTM implementation)
* An extension of TBoot, with the implementation of a dedicated DRTM MLE


Design of the Secure Boot in DRTM mode
======================================

DRTM implementation
-------------------

Regarding the DRTM implementation, we rely on the Intel Trusted Boot
extension combined with a dedicated MLE (Measured Launch
Environment). In Trusted Boot, the associated MLE is responsible for
verifying the integrity of the Linux kernel and its initrd file by
comparing their hash to a reference one. In OpenDTeX, this MLE is
modified to integrate more functionalities (the secure banner, the
unsealing of critical components, etc.). The modified MLE that is
responsible for those new functionalities is called TLoader. We
describe its architecture and usage in the current chapter, after a
brief reminder regarding Trusted Boot.

Design of Trusted Boot
----------------------

In Trusted Boot, the execution flow is as follow:

* A DRTM sequence is initiated by the GETSEC[SENTER] instruction from
  the Intel SMX instruction set.
* The processor measures and then executes an Intel-signed binary
  called SINIT AC, which is responsible for verifying that the
  platform is properly compatible and configured.
* The SINIT AC measures and then executes a component called MLE
  (Measured Launch Environment), which acts as a secure
  loader. Trusted Boot provides its own MLE.
* The Trusted Boot MLE finally measures the Linux kernel and its
  associated initrd and verifies that their hashes matches the
  reference ones. If this is the case, the execution flow is
  transferred to the Linux kernel.


Description of the TLoader MLE
------------------------------

The OpenDTeX MLE, called TLoader, provides the following functionalities:

* The capability to extend the DRTM trust chain.
* The secure banner, to allow explicit local attestation of platform
  integrity to the user.
* The unsealing of files, and especially the Linux kernel and the
  initrd, thus allowing to conditionnaly boot the platform if its
  integrity is conform.

The OpenDTeX TLoader currently supports the following commands,
received through multiboot structure of Grub.

* tpm_loadkey: loads a cryptographic key in the TPM volatile memory.
* tpm_banner: unseals a secure banner (either a text or an image) and
  presents it to the user.
* launch: loads a kernel in memory, measures its integrity and extends
  the result to a specific TPM PCR register.
* initrd: loads an initrd file in memory, measures its integrity and
  extends the result to a specific TPM PCR register.
* tpm_launch: loads an encrypted kernel in memory, unseals it,
  measures its integrity and extends the result to a specific TPM PCR
  register.
* tpm_initrd: loads an encrypted initrd file in memory, unseals it,
  measures its integrity and extends the result to a specific TPM PCR
  register.
* logging: specifies the logging verbosity level.


Usage of the Secure Boot in DRTM mode
=====================================

Create a TPM key:

  $ cd tools/
  $ make
  $ tcg_createkey -k | -z depth key1.key PCR1:PCR2

Seal a secret text message:

  $ echo "My secret message" > /tmp/test
  $ python createStruct.py text /tmp/test > /tmp/test.data
  $ ./tcg_seal -i /tmp/test.data -o data.seal -z -k key1.key

Put the sealed data in the boot directory:

  $ sudo mkdir /boot/opendtex/
  $ sudo cp data.seal /boot/opendtex/data.seal

Or seal a secret image:

  $ python createStruct.py image zoby.bmp > /tmp/test

  $ echo "My secret message" > /tmp/test
  $ python createStruct.py text /tmp/test > /tmp/test.data
  $ ./tcg_seal -i /tmp/test.data -o data.seal -z -k key1.key

Put the sealed data (either the message or the image) in the boot
directory:

  $ sudo mkdir /boot/opendtex/
  $ sudo cp data.seal /boot/opendtex/data.seal


Acknowledgment
==============

We would like to thanks people behind the following projects:
* Intel Trusted Boot: http://sourceforge.net/projects/tboot/
* Flicker: http://sourceforge.net/projects/flickertcb/