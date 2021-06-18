srpm:
    dnf -y install git
    git submodule init
    git submodule update
    rpkg --config dist/rpkg.conf srpm --spec $(spec) --outdir $(outdir)
