package brain

func buildLogics(c Controller) []ControlLogic {
	return buildV1Logics(c)
}

func buildV1Logics(c Controller) []ControlLogic {
	led := MustBuildLogic(NewLiveLEDLogic(c))
	vision := MustBuildLogic(NewVisionLogic(c)).(*VisionLogic)
	camsupp := MustBuildLogic(NewCamSupportLogic(c, vision.Subscribe()))
	return []ControlLogic{led, vision, camsupp}
}
