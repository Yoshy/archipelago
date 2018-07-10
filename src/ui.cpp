#include "ui.h"
#include "game.h"

namespace Archipelago
{
	// UI constants
	const float ui_StatusBarHeight{ 38 };
	const std::string& UI_TOP_STATUSBAR_GAME_TIME_LABEL_ID{ "tsb_time_label" };
	const std::string& UI_TOP_STATUSBAR_GOODS_LABEL_ID{ "tsb_wares_label" };
	const std::string& UI_BOTTOM_STATUSBAR_LABEL_ID{ "bsb_label" };
	const float UI_FPS_UPDATE_INTERVAL{ 1 };
}

using namespace Archipelago;

Ui::Ui(Game* game): _game(game), _fpsUpdateInterval(UI_FPS_UPDATE_INTERVAL), _timeSincelastFpsUpdate(0)
{
	_sfgui = std::make_unique<sfg::SFGUI>();
	_uiDesktop = std::make_unique<sfg::Desktop>();
	_constructTopStatusBar();
	_constructBottomStatusBar();
	_constructTerrainInfoWindow();
	_constructMainInterfaceWindow();
	resizeUi(_game->getRenderWindowWidth(), _game->getRenderWindowHeight());
}

void Ui::render()
{
	_sfgui->Display(_game->getRenderWindow());
}

void Ui::update(float seconds)
{
	_timeSincelastFpsUpdate += seconds;
	if (_timeSincelastFpsUpdate > _fpsUpdateInterval) {
		std::dynamic_pointer_cast<sfg::Label>(_uiBottomStatusBar->GetWidgetById(UI_BOTTOM_STATUSBAR_LABEL_ID))->SetText(_game->getStatusString());
		_timeSincelastFpsUpdate = 0;
	}

	_uiDesktop->Update(seconds);
}

void Ui::updateSettlementWares() {
	for (unsigned int wareIndex = 0; wareIndex < _game->getSettlementWaresNumber(); wareIndex++) {
		auto s = UI_TOP_STATUSBAR_GOODS_LABEL_ID + std::to_string(wareIndex);
		std::dynamic_pointer_cast<sfg::Label>(_uiTopStatusBar->GetWidgetById(s))->SetText(std::to_string(_game->getWareAmount(wareIndex)));
	};
}

void Ui::updateGameTimeString() {
	std::dynamic_pointer_cast<sfg::Label>(_uiTopStatusBar->GetWidgetById(UI_TOP_STATUSBAR_GAME_TIME_LABEL_ID))->SetText(_game->composeGameTimeString());
}

void Ui::handleEvent(const sf::Event& event)
{
	_uiDesktop->HandleEvent(event);
}

void Ui::resizeUi(float width, float height)
{
	_uiTopStatusBar->SetAllocation(sf::FloatRect(0, 0, width, ui_StatusBarHeight));
	std::dynamic_pointer_cast<sfg::Label>(_uiTopStatusBar->GetWidgetById(UI_TOP_STATUSBAR_GAME_TIME_LABEL_ID))->SetAlignment(sf::Vector2f(1.0f, 0.0f));

	_uiBottomStatusBar->SetAllocation(sf::FloatRect(0, height - ui_StatusBarHeight, width, ui_StatusBarHeight));
	std::dynamic_pointer_cast<sfg::Label>(_uiBottomStatusBar->GetWidgetById(UI_BOTTOM_STATUSBAR_LABEL_ID))->SetAlignment(sf::Vector2f(0.0f, 0.0f));

	_uiMainInterfaceWindow->SetAllocation(sf::FloatRect(0, ui_StatusBarHeight, 0, height - 2.0f * ui_StatusBarHeight));

	sf::View v = _game->getRenderWindow().getView();
	v.setSize(width, height);
	_game->getRenderWindow().setView(v);
};

void Ui::showTerrainInfoWindow(sf::Vector2f position, TileComponent& tile) {
	_uiTerrainInfoWindow->setPosition(position);
	_uiTerrainInfoWindow->show(true);
}

void Ui::hideTerrainInfoWindow() {
	_uiTerrainInfoWindow->show(false);
}

void Ui::_constructTopStatusBar() {
	_uiTopStatusBar = sfg::Window::Create();
	_uiTopStatusBar->SetStyle(sfg::Window::BACKGROUND);
	auto currentGameTimeLabel = sfg::Label::Create();
	currentGameTimeLabel->SetId(UI_TOP_STATUSBAR_GAME_TIME_LABEL_ID);
	auto mainBox = sfg::Box::Create(sfg::Box::Orientation::HORIZONTAL, 0.0f);
	auto waresBox = sfg::Box::Create(sfg::Box::Orientation::HORIZONTAL, 0.0f);
	waresBox->SetSpacing(10.0f);
	for (unsigned int wareIndex = 0; wareIndex < _game->getSettlementWaresNumber(); wareIndex++) {
		auto wareIcon = sfg::Image::Create(_game->getWareIcon(wareIndex));
		auto waresAmountText = sfg::Label::Create(std::to_string(_game->getWareAmount(wareIndex)));
		auto spacer = sfg::Label::Create("  ");
		waresAmountText->SetAlignment(sf::Vector2f(0.0f, 0.5f));
		waresAmountText->SetId(UI_TOP_STATUSBAR_GOODS_LABEL_ID + std::to_string(wareIndex));
		waresBox->Pack(wareIcon, false, true);
		waresBox->Pack(waresAmountText, false, true);
		waresBox->Pack(spacer, false, true);
	}
	mainBox->Pack(waresBox);
	mainBox->Pack(currentGameTimeLabel);
	_uiTopStatusBar->Add(mainBox);
	_uiDesktop->Add(_uiTopStatusBar);
}

void Ui::_constructBottomStatusBar() {
	_uiBottomStatusBar = sfg::Window::Create();
	_uiBottomStatusBar->SetStyle(sfg::Window::BACKGROUND);
	auto _uiBottomStatusBarLabel = sfg::Label::Create();
	_uiBottomStatusBarLabel->SetId(UI_BOTTOM_STATUSBAR_LABEL_ID);
	_uiBottomStatusBar->Add(_uiBottomStatusBarLabel);
	_uiDesktop->Add(_uiBottomStatusBar);
}

void Ui::_constructMainInterfaceWindow() {
	_uiMainInterfaceWindow = sfg::Window::Create();
	_uiMainInterfaceWindow->Show(true);
	_uiMainInterfaceWindow->SetStyle(sfg::Window::BACKGROUND);
	auto buildMenu = sfg::Box::Create(sfg::Box::Orientation::VERTICAL, 10.0f);

	sf::Image buildingIconImg;
	std::shared_ptr<sfg::Box> buildingBox;
	auto gamePtr = _game;
	_constructBuildingTipWindow();
	auto bldWindow = _uiBuildingTipWindow.get();
	BuildingSpecification bs;
	for (BuildingTypeId bldId = BuildingTypeId::_First; bldId <= BuildingTypeId::_Last; bldId = static_cast<BuildingTypeId>(std::underlying_type<BuildingTypeId>::type(bldId) + 1)) {
		bs = _game->getAssetRegistry().getBuildingSpecification(bldId);
		buildingIconImg = bs.icon->copyToImage();
		buildingBox = sfg::Box::Create(sfg::Box::Orientation::HORIZONTAL, 10.0f);
		buildingBox->Pack(sfg::Image::Create(buildingIconImg), false);
		buildingBox->Pack(sfg::Label::Create(bs.name), false);
		buildingBox->GetSignal(sfg::Box::OnLeftClick).Connect([gamePtr, bldId] { gamePtr->onUISelectBuilding(bldId); });
		buildingBox->GetSignal(sfg::Box::OnMouseMove).Connect([bldWindow, bldId] { bldWindow->onMouseMove(bldId); });
		buildingBox->GetSignal(sfg::Box::OnMouseLeave).Connect([bldWindow, bldId] { bldWindow->onMouseLeave(); });
		buildMenu->Pack(buildingBox, false);
	}

	auto infoBox = sfg::Box::Create(sfg::Box::Orientation::VERTICAL);

	auto mainNotebook = sfg::Notebook::Create();
	mainNotebook->AppendPage(buildMenu, sfg::Label::Create("Build"));
	mainNotebook->AppendPage(infoBox, sfg::Label::Create("Info"));
	_uiMainInterfaceWindow->Add(mainNotebook);
	_uiDesktop->Add(_uiMainInterfaceWindow);
}

void Ui::_constructTerrainInfoWindow() {
	_uiTerrainInfoWindow = std::make_unique<UiTerrainInfoWindow>(_game);
	_uiDesktop->Add(_uiTerrainInfoWindow->getSFGWindow());
}

void Ui::_constructBuildingTipWindow() {
	_uiBuildingTipWindow = std::make_unique<Archipelago::UiBuildingTipWindow>(_game);
	_uiDesktop->Add(_uiBuildingTipWindow->getSFGWindow());
}