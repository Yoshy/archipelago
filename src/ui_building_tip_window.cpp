#include "ui_building_tip_window.h"
#include "game.h"

using namespace Archipelago;

UiBuildingTipWindow::UiBuildingTipWindow(Archipelago::Game* game) :
	_game(game),
	_isShown(false),
	_window(sfg::Window::Create(sfg::Window::BACKGROUND | sfg::Window::TITLEBAR)) {
	_window->Show(false);
}

void UiBuildingTipWindow::onMouseLeave() {
	_isShown = false;
	_show(false);
}

void UiBuildingTipWindow::onMouseMove(BuildingTypeId buildingId) {
	_buildingId = buildingId;
	_show(true);
}

void UiBuildingTipWindow::_show(bool show) {

	const int BaseZOrder = 1000000;

	if (show && _isShown) {
		_window->SetPosition(sf::Vector2f(sf::Mouse::getPosition(_game->getRenderWindow())) + sf::Vector2f(1.3f * _game->getMouseSprite().getTexture()->getSize().x, 0));
		return;
	}
	if (!show) {
		_window->Show(false);
		return;
	}
	BuildingSpecification bs = _game->getAssetRegistry().getBuildingSpecification(_buildingId);
	_window->RemoveAll();
	_window->SetTitle(bs.name);
	_window->SetPosition(sf::Vector2f(sf::Mouse::getPosition(_game->getRenderWindow())) + sf::Vector2f(1.3f * _game->getMouseSprite().getTexture()->getSize().x, 0));

	auto rootLayoutBox = sfg::Box::Create(sfg::Box::Orientation::VERTICAL, 10.0f);
	_window->Add(rootLayoutBox);

	auto descLabel = sfg::Label::Create(bs.description);
	descLabel->SetZOrder(BaseZOrder + 1);
	rootLayoutBox->Pack(descLabel);

	auto sep = sfg::Separator::Create();
	sep->SetZOrder(BaseZOrder + 1);
	rootLayoutBox->Pack(sep);

	auto resNeedsLabel = sfg::Label::Create("Required natural resources: " + _game->getAssetRegistry().getNatresSpecification(bs.natresRequired).name);
	resNeedsLabel->SetAlignment({ 0.0f, 0.0f });
	resNeedsLabel->SetZOrder(BaseZOrder + 1);
	rootLayoutBox->Pack(resNeedsLabel);

	auto waresNeedsLabel = sfg::Label::Create("Required wares:");
	waresNeedsLabel->SetAlignment({ 0.0f, 0.0f });
	waresNeedsLabel->SetZOrder(BaseZOrder + 1);
	rootLayoutBox->Pack(waresNeedsLabel);

	auto wares = bs.waresRequired;
	if (wares.size() == 0) {
		waresNeedsLabel = sfg::Label::Create("None");
		waresNeedsLabel->SetAlignment({ 0.0f, 0.0f });
		waresNeedsLabel->SetZOrder(BaseZOrder + 1);
		rootLayoutBox->Pack(waresNeedsLabel);
	}
	for (WaresStack ware : wares) {
		WaresSpecification wareSpec = _game->getAssetRegistry().getWaresSpecification(ware.type);
		int amount = ware.amount;
		auto wareHLayoutBox = sfg::Box::Create(sfg::Box::Orientation::HORIZONTAL, 10.0f);

		auto wareIcon = sfg::Image::Create();
		wareIcon->SetImage(wareSpec.icon->copyToImage());
		wareIcon->SetZOrder(BaseZOrder + 1);
		wareIcon->SetAlignment({ 0.0f, 0.0f });
		wareHLayoutBox->Pack(wareIcon, false);

		auto wareAmountLabel = sfg::Label::Create(std::to_string(amount));
		wareAmountLabel->SetZOrder(BaseZOrder + 1);
		wareAmountLabel->SetAlignment({ 0.0f, 0.0f });
		wareHLayoutBox->Pack(wareAmountLabel, false);

		if (!_game->settlementHasWareForBuilding(bs, ware.type)) {
			auto warnIcon = sfg::Image::Create();
			warnIcon->SetImage(_game->getAssetRegistry().getTexture("triangle_atention")->copyToImage());
			warnIcon->SetZOrder(BaseZOrder + 1);
			warnIcon->SetAlignment({ 0.0f, 0.0f });
			wareHLayoutBox->Pack(warnIcon, false);
		}

		rootLayoutBox->Pack(wareHLayoutBox);
	}

	_isShown = true;
	_window->Show(true);
	_window->SetZOrder(BaseZOrder);
}
