#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Texture.hpp>
#include "ui_terrain_info_window.h"
#include "game.h"

using namespace Archipelago;

UiTerrainInfoWindow::UiTerrainInfoWindow(Game* game) :
	_game(game),
	_window(sfg::Window::Create(sfg::Window::BACKGROUND | sfg::Window::TITLEBAR)),
	_hTileSprite(sfg::Image::Create()),
	_hTileName(sfg::Label::Create("TileName"))
{
	_game->getWorld()->subscribe<TerrainInfoWindowDataUpdateEvent>(this);

	_window->SetTitle("Terrain info");
	_window->SetRequisition({ 100.0f, 0.0f });
	_window->Show(false);
}

UiTerrainInfoWindow::~UiTerrainInfoWindow() {
	_game->getWorld()->unsubscribe<TerrainInfoWindowDataUpdateEvent>(this);
}

void UiTerrainInfoWindow::receive(ECS::World* world, const TerrainInfoWindowDataUpdateEvent& event) {
	spdlog::get(loggerName)->trace("TerrainInfoWindowDataUpdateEvent received");
	if (event.show == false) {
		show(false);
		return; // Don't update window data for hidden window
	}
	auto rootLayoutBox = sfg::Box::Create(sfg::Box::Orientation::VERTICAL, 10.0f);
	auto tileInfoBox = sfg::Box::Create(sfg::Box::Orientation::HORIZONTAL, 10.0f);
	rootLayoutBox->Pack(tileInfoBox);

	tileInfoBox->Pack(_hTileSprite, false);
	tileInfoBox->Pack(_hTileName, false);
	sf::Image img = (event.tileSprite->getTexture())->copyToImage();
	_hTileSprite->SetImage(img);
	_hTileName->SetText(event.name);
	
	if (event.tileType == TileType::BUILDING) {
		_addBuildingInfoLayout(rootLayoutBox, event);
	}
	else {
		_addTerrainInfoLayout(rootLayoutBox, event);
	}
	_window->RemoveAll();
	_window->SetPosition(event.position);
	_window->Add(rootLayoutBox);
	show(true);
}

void UiTerrainInfoWindow::_addBuildingInfoLayout(sfg::Box::Ptr rootLayoutWidget, const TerrainInfoWindowDataUpdateEvent& event) {
	auto buildingDescription = sfg::Box::Create(sfg::Box::Orientation::HORIZONTAL, 10.0f);
	auto descLabel = sfg::Label::Create(event.buildingDescription);
	descLabel->SetLineWrap(true);
	descLabel->SetRequisition(sf::Vector2f(BUILDING_DESCRIPTION_LABEL_REQUISITION, 0.0f));
	descLabel->SetAlignment(sf::Vector2f(0.0f, 0.0f));
	buildingDescription->Pack(descLabel);
	rootLayoutWidget->Pack(buildingDescription);
	rootLayoutWidget->Pack(sfg::Separator::Create(sfg::Separator::Orientation::HORIZONTAL));

	auto bpl = sfg::Label::Create("Building production per month:");
	bpl->SetAlignment(sf::Vector2f(0.0f, 0.0f));
	rootLayoutWidget->Pack(bpl);
	for (WaresStack ws : *event.production) {
		auto productionInfoHBox = sfg::Box::Create(sfg::Box::Orientation::HORIZONTAL, 10.0f);
		auto prodTypeLabel{ sfg::Label::Create(_game->getAssetRegistry().getWaresSpecification(ws.type).name) };
		prodTypeLabel->SetAlignment(sf::Vector2f(0.0f, 0.0f));
		productionInfoHBox->Pack(prodTypeLabel);
		rootLayoutWidget->Pack(productionInfoHBox);
		auto prodAmountLabel{ sfg::Label::Create(std::to_string(ws.amount)) };
		prodAmountLabel->SetAlignment(sf::Vector2f(0.0f, 0.0f));
		productionInfoHBox->Pack(prodAmountLabel);
	}
}

void UiTerrainInfoWindow::_addTerrainInfoLayout(sfg::Box::Ptr rootLayoutWidget, const TerrainInfoWindowDataUpdateEvent& event) {
	rootLayoutWidget->Pack(sfg::Separator::Create(sfg::Separator::Orientation::HORIZONTAL));

	uint32_t resourceSet = event.resourceSet;
	uint32_t mask = 0x000000FF;
	auto natresHBox = sfg::Box::Create(sfg::Box::Orientation::HORIZONTAL, 10.0f);
	natresHBox->Pack(sfg::Label::Create("Available resources on this land:"));
	rootLayoutWidget->Pack(natresHBox);
	for (unsigned int g = 0; g < 4; g++) {
		int numWares = 1;
		NaturalResourceTypeId natresType = static_cast<NaturalResourceTypeId>((resourceSet & mask) >> (g * 8));
		mask = mask << 8;
		if ((natresType >= NaturalResourceTypeId::_First) && (natresType <= NaturalResourceTypeId::_Last)) {
			auto natresImage = sfg::Image::Create();
			auto natresName = sfg::Label::Create();
			auto natresHBox = sfg::Box::Create(sfg::Box::Orientation::HORIZONTAL, 10.0f);
			rootLayoutWidget->Pack(natresHBox);
			sf::Image i = _game->getAssetRegistry().getNatresSpecification(natresType).icon->copyToImage();
			natresImage->SetImage(i);
			natresImage->SetAlignment(sf::Vector2f(0.0f, 0.0f));
			natresHBox->Pack(natresImage);
			natresName->SetText(_game->getAssetRegistry().getNatresSpecification(natresType).name);
			natresImage->SetAlignment(sf::Vector2f(0.0f, 0.0f));
			natresHBox->Pack(natresName);
		}
	}

}
