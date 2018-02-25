#include "ui.h"
#include "game.h"

namespace Archipelago
{
	// UI constants
	const float ui_StatusBarHeight{ 38 };
	const std::string& ui_TopStatusBar_TimeLabelId{ "tsb_time_label" };
	const std::string& ui_TopStatusBar_GoodsLabelId{ "tsb_wares_label" };
	const std::string& ui_BottomStatusBar_LabelId{ "bsb_label" };
}

using namespace Archipelago;

Ui::Ui(Game& game): _game(game)
{
	_sfgui = std::make_unique<sfg::SFGUI>();
	_uiDesktop = std::make_unique<sfg::Desktop>();
	_constructTopStatusBar();
	_constructBottomStatusBar();
	_constructMainInterfaceWindow();
	_constructTerrainInfoWindow();
	resizeUi(_game.getRenderWindowWidth(), _game.getRenderWindowHeight());
}

void Ui::render()
{
	_sfgui->Display(_game.getRenderWindow());
}

void Ui::update(float seconds)
{
	// Update game time text
	std::dynamic_pointer_cast<sfg::Label>(_uiTopStatusBar->GetWidgetById(ui_TopStatusBar_TimeLabelId))->SetText(_game.composeGameTimeString());
	// Update status bar
	std::dynamic_pointer_cast<sfg::Label>(_uiBottomStatusBar->GetWidgetById(ui_BottomStatusBar_LabelId))->SetText(_game.getStatusString());
	// Update amount of wares
	for (unsigned int wareIndex = 0; wareIndex < _game.getSettlementWaresNumber(); wareIndex++) {
		auto s = ui_TopStatusBar_GoodsLabelId + std::to_string(wareIndex);
		std::dynamic_pointer_cast<sfg::Label>(_uiTopStatusBar->GetWidgetById(s))->SetText(std::to_string(_game.getWareAmount(wareIndex)));
	};

	_uiDesktop->Update(seconds);
}

void Ui::handleEvent(const sf::Event& event)
{
	_uiDesktop->HandleEvent(event);
}

void Ui::resizeUi(float width, float height)
{
	_uiTopStatusBar->SetAllocation(sf::FloatRect(0, 0, width, ui_StatusBarHeight));
	std::dynamic_pointer_cast<sfg::Label>(_uiTopStatusBar->GetWidgetById(ui_TopStatusBar_TimeLabelId))->SetAlignment(sf::Vector2f(1.0f, 0.0f));

	_uiBottomStatusBar->SetAllocation(sf::FloatRect(0, height - ui_StatusBarHeight, width, ui_StatusBarHeight));
	std::dynamic_pointer_cast<sfg::Label>(_uiBottomStatusBar->GetWidgetById(ui_BottomStatusBar_LabelId))->SetAlignment(sf::Vector2f(0.0f, 0.0f));

	_uiMainInterfaceWindow->SetAllocation(sf::FloatRect(0, ui_StatusBarHeight, 0, height - 2.0f * ui_StatusBarHeight));

	sf::View v = _game.getRenderWindow().getView();
	v.setSize(width, height);
	_game.getRenderWindow().setView(v);
};

void Ui::showTerrainInfoWindow(sf::Vector2f position, TileComponent& tile) {
	_uiTerrainInfoWindow->SetPosition(position);
	_uiTerrainInfoWindow->Show(true);
}

void Ui::hideTerrainInfoWindow() {
	_uiTerrainInfoWindow->Show(false);
}

void Ui::_constructTopStatusBar() {
	_uiTopStatusBar = sfg::Window::Create();
	_uiTopStatusBar->SetStyle(sfg::Window::BACKGROUND);
	auto currentGameTimeLabel = sfg::Label::Create();
	currentGameTimeLabel->SetId(ui_TopStatusBar_TimeLabelId);
	auto mainBox = sfg::Box::Create(sfg::Box::Orientation::HORIZONTAL, 0.0f);
	auto waresBox = sfg::Box::Create(sfg::Box::Orientation::HORIZONTAL, 0.0f);
	waresBox->SetSpacing(10.0f);
	for (unsigned int wareIndex = 0; wareIndex < _game.getSettlementWaresNumber(); wareIndex++) {
		auto wareIcon = sfg::Image::Create(_game.getWareIcon(wareIndex));
		auto waresAmountText = sfg::Label::Create(std::to_string(_game.getWareAmount(wareIndex)));
		auto spacer = sfg::Label::Create("  ");
		waresAmountText->SetAlignment(sf::Vector2f(0.0f, 0.5f));
		waresAmountText->SetId(ui_TopStatusBar_GoodsLabelId + std::to_string(wareIndex));
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
	_uiBottomStatusBarLabel->SetId(ui_BottomStatusBar_LabelId);
	_uiBottomStatusBar->Add(_uiBottomStatusBarLabel);
	_uiDesktop->Add(_uiBottomStatusBar);
}

void Ui::_constructMainInterfaceWindow() {
	_uiMainInterfaceWindow = sfg::Window::Create();
	_uiMainInterfaceWindow->Show(true);
	_uiMainInterfaceWindow->SetStyle(sfg::Window::BACKGROUND);
	//_uiMainInterfaceWindow->GetSignal(sfg::Window::OnMouseEnter).Connect(std::bind([this] { _onTerrainInfoWindowMouseEnter(); }));
	//_uiMainInterfaceWindow->GetSignal(sfg::Window::OnMouseLeave).Connect(std::bind([this] { _onTerrainInfoWindowMouseLeave(); }));
	auto buildBox = sfg::Box::Create(sfg::Box::Orientation::VERTICAL, 10.0f);

	sf::Image buildingIconImg;
	std::shared_ptr<sfg::Box> buildingBox;
	Game* gamePtr = &_game;
	BuildingSpecification bs;
	for (BuildingTypeId bldId = BuildingTypeId::_First; bldId <= BuildingTypeId::_Last; bldId = static_cast<BuildingTypeId>(std::underlying_type<BuildingTypeId>::type(bldId) + 1)) {
		bs = gamePtr->getAssetRegistry().getBuildingSpecification(bldId);
		buildingIconImg = bs.icon->copyToImage();
		buildingBox = sfg::Box::Create(sfg::Box::Orientation::HORIZONTAL, 10.0f);
		buildingBox->Pack(sfg::Image::Create(buildingIconImg), false);
		buildingBox->Pack(sfg::Label::Create(bs.name), false);
		buildingBox->GetSignal(sfg::Box::OnLeftClick).Connect([gamePtr, bldId] {gamePtr->onUISelectBuilding(bldId); });
		buildBox->Pack(buildingBox, false);
	}

	auto infoBox = sfg::Box::Create(sfg::Box::Orientation::VERTICAL);

	auto mainNotebook = sfg::Notebook::Create();
	mainNotebook->AppendPage(buildBox, sfg::Label::Create("Build"));
	mainNotebook->AppendPage(infoBox, sfg::Label::Create("Info"));
	_uiMainInterfaceWindow->Add(mainNotebook);
	_uiDesktop->Add(_uiMainInterfaceWindow);
}

void Ui::_constructTerrainInfoWindow() {
	_uiTerrainInfoWindow = sfg::Window::Create(sfg::Window::BACKGROUND | sfg::Window::TITLEBAR);
	_uiTerrainInfoWindow->SetTitle("Terrain info");
	_uiTerrainInfoWindow->SetRequisition({ 100.0f, 0.0f });
	_uiTerrainInfoWindow->Show(false);
	_uiDesktop->Add(_uiTerrainInfoWindow);
}