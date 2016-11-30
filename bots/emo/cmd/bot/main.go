package main

import (
	"fmt"

	"github.com/codingbrain/clix.go/exts/bind"
	"github.com/codingbrain/clix.go/exts/help"
	"github.com/codingbrain/clix.go/term"
)

// Version is the release version
const Version = "0.0.1"

// VersionSuffix is the suffix of version
var VersionSuffix = "-dev"

type versionCmd struct {
}

func (c *versionCmd) Execute(args []string) error {
	fmt.Println(Version + VersionSuffix)
	return nil
}

func main() {
	cliDef.
		Use(term.NewExt()).
		Use(bind.NewExt().
			Bind(&boardCmd{}, "board").
			Bind(&versionCmd{}, "version")).
		Use(help.NewExt()).
		Parse().
		Exec()
}
