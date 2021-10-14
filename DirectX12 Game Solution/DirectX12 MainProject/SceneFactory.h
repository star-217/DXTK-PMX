//
// SceneFactory.h
//

#include "MainScene.h"

class SceneFactory final {
public:
	static std::unique_ptr<Scene> CreateScene(const NextScene nextScene)
	{
		std::unique_ptr<Scene> scene;
		switch (nextScene) {
		case NextScene::MainScene:	scene = std::make_unique<MainScene>();	break;
		}
		return scene;
	}
};