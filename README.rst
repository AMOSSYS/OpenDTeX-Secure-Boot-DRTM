====================
OpenDTeX Secure Boot
====================

Project Description
===================

The OpenDTeX research project aims at providing trusted building
blocks to ensure strong security properties during the boot chain and
to allow secure execution of isolated enclaves on x86
architectures. OpenDTeX has been achieved with the help of the French
“`RAPID <http://www.ixarm.com/Projets-d-innovation-duale-RAPID>`_”
grant process, which targets both civil and defense use cases, through
a consortium composed of AMOSSYS, Bertin Technologies and Telecom
ParisTech.

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

Authors and Sponsors
====================

See the top distribution file ``AUTHORS.txt`` for the detailed and updated list
of authors.

Project sponsors:

* AMOSSYS: `http://www.amossys.fr <http://www.amossys.fr>`_
* Bertin Technologies: `http://www.bertin.fr <http://www.bertin.fr>`_
* Telecom ParisTech: `https://www.telecom-paristech.fr <https://www.telecom-paristech.fr>`_

License
=======

This software is licensed under the Modified BSD License. See the
``COPYING.txt`` file for the full license text.

More Information
================

:Website: `https://github.com/AMOSSYS/OpenDTeX-Secure-Boot-DRTM <https://github.com/AMOSSYS/OpenDTeX-Secure-Boot-DRTM>`_
:Email: `etudes@amossys.fr <etudes@amossys.fr>`_
:Twitter: Follow AMOSSYS's official accounts (`@amossys <https://twitter.com/Amossys>`_)

OpenDTeX developments
=====================

OpenDTeX work notably include:

* A TPM 1.2 API library independent from the BIOS or OS
* A minimal TSS API library independent from the OS
* A set of tools to manipulate the TPM.
* An extension of Grub 2 (i.e. an SRTM implementation)
* An extension of TBoot, with the implementation of a dedicated DRTM MLE

Comparison with similar tools
=============================

* Trusted Grub: this tool permits to extend the trust chain
  by measuring components that are executed beyond the BIOS, in SRTM mode.
* Trusted Grub: this tool permits to start a new trust chain, in DRTM
  mode. Is also permits to verify the integrity of the Linux kernel
  and its associated initrd.
* Bitlocker (in TPM mode): this tools allows to seal the master key
  with the TPM so that decryption is possible only if the platform
  integrity is correct. It only works through a SRTM (which means a large
  TCB).
* Anti-Evil-Maid proof of concept from Joanna Rutkowska, which
  implements the concept of secure banner. This PoC only supports
  SRTM.

OpenDTeX Secure Boot allows both integrity verification and unsealing
of boot time components, either in SRTM or DRTM mode. Besides, it
provides explicit trust attestation to the user thanks to a shared
secret (the secret banner).


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


The Secure Banner design
------------------------

As mentionned above, the secure banner principle intends to provide
explicit local attestation of the platform integrity to the user. The
secure banner is in fact a message or image only known from the user
and encrypted with a TPM cryptographic key, so that it can only be
decrypted if the platform integrity is what is expected by the
user. Thus, this text or image have to be sealed in order to
caracterize the integrity of the plateform. In other words, at boot
time, if the secure banner can be unsealed, this means that the
platform integrity is correct.


Hardware requirements
---------------------

Secure Boot component requires some hardware support:

* A TPM 1.2 (Trusted Platform Module).
* A chipset that supports Intel TXT and IOMMU.
* A processor that supports Intel TXT (through the SMX instruction set) and VT-x.

Basically, a platform that have an Intel VPro sticker should be compatible.

Besides those hardware requirements, they have to be activated in the BIOS.


Compilation and installation of the DRTM components
---------------------------------------------------

This chapter presents the different steps required to compile and install the OpenDTeX Secure Boot components in DRTM mode, which are:

* libtpm : a library that provides an API to communicate with the TPM.
* libuc : a library that provides basic libc functionalities such as memory and strings handling.
* libuvideo : a library that provides direct access to the video card framebuffer.
* libtxt : a library that exposes functionalities linked with the Intel SMX instruction set and management of TXT heap/registers.
* Trusted Boot : we rely on Trusted Boot for the preparation and launch of the DRTM environemnt.
* AC SINIT : a signed binary provided by Intel, which is responsible for verifying that the platform is properly compatible and configured.
* Tloader : a library that implements a MLE (i.e. the component that is called by the AC SINIT during a DRTM) and provides services such as the secure banner and the unsealing of critical components.


### Dependencies

In order to compile the OpenDTeX Secure Boot components, you should have:

* make
* gcc
* autoconf
* automake
* m4
* autotools-dev

### Compilation and installation of libuc

In "libuc/"::

  $ ./autogen.sh
  $ ./configure --prefix=/opt/ulib
  $ make
  $ sudo make install

### Compilation and installation of uvideo

In "libuvideo/"::

  $ ./autogen.sh
  $ PKG_CONFIG_PATH=/opt/ulib/lib/pkgconfig ./configure --prefix=/opt/ulib
  $ make
  $ sudo make install

### Compilation and installation of libtpm

In "libtpm/"::

  $ ./autogen.sh
  $ PKG_CONFIG_PATH=/opt/ulib/lib/pkgconfig ./configure --prefix=/opt/ulib
  $ make
  $ sudo make install

### Compilation and installation of libtxt

In "libtxt/"::

  $ ./autogen.sh
  $ PKG_CONFIG_PATH=/opt/ulib/lib/pkgconfig ./configure --prefix=/opt/ulib
  $ make
  $ sudo make install

### Compilation and installation of Tloader

In "tloader/"::

  $ ./autogen.sh
  $ PKG_CONFIG_PATH=/opt/ulib/lib/pkgconfig ./configure
  $ make

This last step will generate a file tloader.gz. You have to place this
file in the directory "/boot/".

### Compilation and installation of Trusted Boot

Trusted Boot in available on Sourceforge: http://sourceforge.net/projects/tboot/

Once the archive has been downloaded, go inside the tboot/ directory, and then::

  $ make
  $ sudo make install

This last step will copy the file tboot.gz (the loader) in the directory "/boot/".

### Deployment of the AC SINIT module

You have to retrieve the AC SINIT module that is compatible with your
chipset/processor on Intel website: http://software.intel.com/en-us/articles/intel-trusted-execution-technology.

The AC SINIT module have to be copied in the directory "/boot/".


Preparation of the Secure Banner
--------------------------------

Create a TPM key, such that it can be loaded again inside the TPM
memory if the PCR1 and PCR2 have the same content that they had during
key creation::

  $ cd tools/
  $ make
  $ tcg_createkey -k | -z depth key1.key PCR1:PCR2

Seal a secret text message with this TPM key, and tell the TPM to seal
the object so that it can be decrypted again if contents of PCR17,
PCR18 and PCR19 have not changed::

  $ echo "My secret message" > /tmp/test
  $ python createStruct.py text /tmp/test > /tmp/test.data
  $ ./tcg_seal -i /tmp/test.data -o data.seal -z -k key1.key -p 17:18:19
  (0000)-> SRK(WellKnown : 1)
  (0001)-> key1.key(WellKnown : 1)(00000000)
  | child of SRK
  symmetric key : b0 49 e5 34 9b f1 c5 59 d3 b5 82 03 58 68 9f a2 f1 ad e4 d3 1c dd 18 bb 01

Or seal a secret image::

  $ python createStruct.py image zoby.bmp > /tmp/test.data #####################################################################################################x à modifier dans README sur github
  $ ./tcg_seal -i /tmp/test.data -o data.seal -z -k key1.key

Put the sealed data (either the message or the image) in the boot directory::

  $ sudo mkdir /boot/opendtex/
  $ sudo cp data.seal /boot/opendtex/data.seal


Configuration of Grub
---------------------

Here is a Grub 2 configuration file example, that permits to launch Trusted Boot, then to execute a DRTM, and then to give execution flow to the TLoader MLE. The TLoader then interpret the command::

  menuentry 'TBoot+TLoader' {
    # GRUB2 modules
    insmod gzio
    insmod part_msdos
    insmod ext2
    # To find system disk boot partition
    set root='(/dev/sda,msdos5)'
    search --no-floppy --fs-uuid --set=root 86cf0374-fbf3-4d36-9b5c-45303f24c17a

    # TrustedBoot main module to start
    multiboot /boot/tboot.gz /boot/tboot.gz logging=vga,memory,serial

    # The platform specific AC module
    module /boot/SINIT

    # TLoader module launched by TrustedBoot
    module /boot/tloader.gz /boot/tloader.gz logging=vga

    # Entry to display the secure banner (with a key to load)
    module /boot/opendtex/key1.key /boot/opendtex/key1.key tpm_loadkey aliasKey1 aliasSRK
    module /boot/opendtex/data.seal /boot/opendtex/data.seal tpm_banner aliasKey1

    # The OS image to start
    module /boot/vmlinuz /boot/vmlinuz kdb=fr vga=791 root=/dev/hda1 ...
    module /boot/initrd /boot/initrd
  }

Now that the platform is correctly configured, the next time you start
the system, a Secure Boot will be launched, meaning that many boot
components will be measured, and if nothing has changed in terms of
integrity, you will see the secure banner.

And if one boot component appears to have its integrity altered, you
will get a error when the TPM attempts to unseal either the protected
key or the protected object.


Acknowledgment
==============

We would like to thanks people behind the following projects:

* Intel Trusted Boot: http://sourceforge.net/projects/tboot/
* Flicker: http://sourceforge.net/projects/flickertcb/
