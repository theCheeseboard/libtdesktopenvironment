srpm:
    dnf -y install git
    git -C $(spec) submodule init
    git -C $(spec) submodule update
    rpkg srpm --spec $(spec) --outdir $(outdir)
    
