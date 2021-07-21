# AutoFix-MKVC
is a C++ written tool which helps with your [mer-kernel-verify-config] errors/warnings!

### The tool will perform [mer-kernel-verify-config], parse output, get a copy of target kernel config file, modify it and save in a special folder.
As simple as 2+2... *Just specify* `-t={kernel-cfg-path}` and there you go!

### Avaliable argument options:
*  `-v` `--verbose`           Outputs every changed variable to give more clarity on the ongoing process.
*  `-e` `--errors-only`       The script will only fix ERROR entries from mer-kernel-verify-config
*  `-t` `--target`            Sets variable for the config that's to be checked, follof argument by "=" and type kernel config path.
*  `-h` `--help`              Outputs this command and quits.
