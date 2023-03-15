#include <utility>


#include "halley/ui/widgets/ui_tree_list.h"

#include <cassert>


#include "halley/utils/algorithm.h"
#include "halley/ui/widgets/ui_image.h"
#include "halley/ui/widgets/ui_label.h"
using namespace Halley;


UITreeList::UITreeList(String id, UIStyle style)
	: UIList(std::move(id), std::move(style))
{
	setupEvents();
}

UITreeListItem& UITreeList::addTreeItem(const String& id, const String& parentId, size_t childIndex, const LocalisedString& label, const String& labelStyleName, Sprite icon, bool forceLeaf, bool expanded)
{
	const auto& style = styles.at(0);
	auto listItem = std::make_shared<UIListItem>(id, *this, style.getSubStyle("item"), int(getNumberOfItems()), style.getBorder("extraMouseBorder"));

	// Controls
	const auto treeControls = std::make_shared<UITreeListControls>(id, style.getSubStyle("controls"));
	listItem->add(treeControls, 0, {}, UISizerFillFlags::Fill);

	// Icon
	const auto root = std::make_shared<UIWidget>("root", Vector2f(), UISizer());
	std::shared_ptr<UIImage> iconWidget;
	if (icon.hasMaterial()) {
		iconWidget = std::make_shared<UIImage>(icon);
		root->add(iconWidget, 0, {}, UISizerAlignFlags::Centre);
	}

	// Label
	const auto& labelStyle = style.getSubStyle(labelStyleName);
	auto labelWidget = std::make_shared<UILabel>(id + "_label", labelStyle, labelStyle.getTextRenderer("normal"), label);
	if (labelStyle.hasTextRenderer("selected")) {
		labelWidget->setSelectable(labelStyle.getTextRenderer("normal"), labelStyle.getTextRenderer("selected"), true);
	}
	if (labelStyle.hasTextRenderer("disabled")) {
		labelWidget->setDisablable(labelStyle.getTextRenderer("normal"), labelStyle.getTextRenderer("disabled"));
	}
	root->add(labelWidget, 0, style.getBorder("labelBorder"), UISizerFillFlags::Fill);

	listItem->add(root, 1);
	listItem->setDraggableSubWidget(root.get());

	// Logical item
	auto treeItem = std::make_unique<UITreeListItem>(id, listItem, treeControls, labelWidget, iconWidget, forceLeaf, expanded);
	auto& parentItem = getItemOrRoot(parentId);
	auto& result = parentItem.addChild(std::move(treeItem), childIndex);

	addItem(listItem, Vector4f(), UISizerAlignFlags::Left | UISizerFillFlags::FillVertical);
	needsRefresh = true;

	return result;
}

void UITreeList::removeItem(const String& id, bool immediate)
{
	auto item = root.removeFromTree(id);
	if (item) {
		removeTree(*item);
	}

	if (immediate) {
		refresh();
	} else {
		needsRefresh = true;
	}
}

void UITreeList::removeTree(const UITreeListItem& tree)
{
	getSizer().remove(*tree.getListItem());
	removeChild(*tree.getListItem());

	items.erase(std::remove_if(items.begin(), items.end(), [&] (const std::shared_ptr<UIListItem>& i)
	{
		return i->getId() == tree.getId();
	}), items.end());

	for (auto& subTree: tree.getChildren()) {
		removeTree(*subTree);
	}
}

void UITreeList::setLabel(const String& id, const LocalisedString& label, Sprite icon)
{
	auto item = root.tryFindId(id);
	if (item) {
		item->setLabel(label);
		item->setIcon(std::move(icon));
	}
}

void UITreeList::setForceLeaf(const String& id, bool forceLeaf)
{
	auto item = root.tryFindId(id);
	if (item) {
		item->setForceLeaf(forceLeaf);
	}
}

bool UITreeList::setSelectedOptionIds(gsl::span<const String> ids, SelectionMode mode)
{
	for (const auto& id: ids) {
		makeParentsOfItemExpanded(id);
	}

	return UIList::setSelectedOptionIds(ids, mode);
}

UITreeListItem* UITreeList::tryGetTreeItem(const String& id)
{
	return root.tryFindId(id);
}

void UITreeList::clear()
{
	UIList::clear();
	root = UITreeListItem();
	needsRefresh = true;
}

void UITreeList::update(Time t, bool moved)
{
	UIList::update(t, moved);
	if (needsRefresh) {
		refresh();
	}
}

void UITreeList::refresh()
{
	const auto sel = getSelectedOptionId();

	root.updateTree(*this);

	reassignIds();

	const auto toSel = root.getLastExpandedItem(sel);
	if (toSel) {
		setSelectedOptionId(toSel.value(), SelectionMode::AddToSelect);
	}

	resetSelectionIfInvalid();

	needsRefresh = false;
}

void UITreeList::draw(UIPainter& painter) const
{
	UIList::draw(painter);
	if (insertCursor.hasMaterial()) {
		painter.draw(insertCursor);
	}
}

void UITreeList::onItemDragging(UIListItem& item, int index, Vector2f pos)
{
	auto elem = root.tryFindId(item.getId());
	if (elem) {
		elem->setExpanded(false);
	}

	const auto res = root.findPosition(*this, pos + item.getRect().getSize() / 2);
	if (res) {
		const auto& resData = res.value();
		auto rect = resData.rect;
		if (rect.getHeight() < 1) {
			rect = Rect4f(rect.getTopLeft() - Vector2f(0, 1), rect.getTopRight() + Vector2f(0, 1));
		}
		
		insertCursor = styles.at(0).getSubStyle("cursor").getSprite(resData.type == UITreeListItem::PositionType::OnTop ? "over" : "beforeAfter");
		insertCursor.setPos(rect.getTopLeft()).scaleTo(rect.getSize());
	}

	sendEvent(UIEvent(UIEventType::MakeAreaVisibleContinuous, getId(), Rect4f(pos, pos + item.getSize()) - getPosition()));
}

void UITreeList::onItemDoneDragging(UIListItem& item, int index, Vector2f pos)
{
	auto res = root.findPosition(*this, pos + item.getRect().getSize() / 2);
	if (res) {
		const auto& resData = res.value();

		String newParentId;
		size_t newChildIndex;
		
		if (resData.type == UITreeListItem::PositionType::OnTop) {
			newParentId = resData.item->getId();
			newChildIndex = resData.item->getNumberOfChildren();
		} else if (resData.type == UITreeListItem::PositionType::End) {
			newParentId = "";
			newChildIndex = root.getNumberOfChildren();
		} else {
			newParentId = resData.item->getParentId();
			const auto parent = root.tryFindId(newParentId);
			const auto siblingIndex = parent->getChildIndex(resData.item->getId());
			newChildIndex = siblingIndex + (resData.type == UITreeListItem::PositionType::Before ? 0 : 1);
		}
		
		reparentItems(getSelectedOptionIds(), newParentId, static_cast<int>(newChildIndex));
	}
	insertCursor = Sprite();
}

bool UITreeList::canParentItemTo(const String& itemId, const String& parentId) const
{
	return true;
}

bool UITreeList::canDragItemId(const String& itemId) const
{
	return true;
}

UITreeListItem& UITreeList::getItemOrRoot(const String& id)
{
	const auto res = root.tryFindId(id);
	if (res) {
		return *res;
	}
	return root;
}

void UITreeList::setupEvents()
{
	setHandle(UIEventType::TreeCollapseHandle, [=] (const UIEvent& event)
	{
		auto elem = root.tryFindId(event.getStringData());
		if (elem) {
			elem->setExpanded(false);
		}
		needsRefresh = true;
		sendEvent(UIEvent(UIEventType::TreeItemExpanded, getId(), event.getStringData(), false));
	});

	setHandle(UIEventType::TreeExpandHandle, [=](const UIEvent& event)
	{
		auto elem = root.tryFindId(event.getStringData());
		if (elem) {
			elem->setExpanded(true);
		}
		needsRefresh = true;
		sendEvent(UIEvent(UIEventType::TreeItemExpanded, getId(), event.getStringData(), true));
	});
}

void UITreeList::reparentItems(gsl::span<const String> itemIds, const String& newParentId, int origNewChildIndex)
{
	ConfigNode::SequenceType reparentNode;
	int newChildIndex = origNewChildIndex;
	int curOffset = 0;

	for (const auto& itemId: itemIds) {
		if (itemId == newParentId || !canParentItemTo(itemId, newParentId)) {
			continue;
		}

		const auto& curItem = *root.tryFindId(itemId);
		const String oldParentId = curItem.getParentId(); // Don't store a reference
		auto& oldParent = *root.tryFindId(oldParentId);
		const size_t oldChildIndex = static_cast<int>(oldParent.getChildIndex(itemId));

		if (oldParentId != newParentId || oldChildIndex != newChildIndex) {
			if (oldParentId == newParentId) {
				oldParent.moveChild(oldChildIndex, newChildIndex);

				// This requires some explanation:
				// The index returned here assumes that the item is still present, so it's inflated by one if moving forwards in the same child
				// We'll accumulate all those offsets first, then apply to all entities at the end
				if (newChildIndex > oldChildIndex) {
					--curOffset;
				}
			} else {
				auto& newParent = *root.tryFindId(newParentId);
				newParent.addChild(oldParent.removeChild(itemId), newChildIndex);
			}

			auto& entry = reparentNode.emplace_back(ConfigNode::MapType());
			entry["itemId"] = itemId;
			entry["parentId"] = newParentId;
			entry["oldParentId"] = oldParentId;
			entry["childIdx"] = newChildIndex;
			entry["oldChildIdx"] = static_cast<int>(oldChildIndex);

			++newChildIndex;
		}
	}

	if (!reparentNode.empty()) {
		if (curOffset != 0) {
			for (auto& e: reparentNode) {
				e["childIdx"] = e["childIdx"].asInt() + curOffset;
			}
		}

		sortItems();
		needsRefresh = true;

		sendEvent(UIEvent(UIEventType::TreeItemReparented, getId(), std::move(reparentNode)));
	}
}

void UITreeList::sortItems()
{
	// Store previous curOption
	setCanSendEvents(false);
	const auto oldOption = getSelectedOptionId();
	
	// Update list representation
	items.clear();
	root.collectItems(items);
	reassignIds();

	// Restore curOption
	setSelectedOptionId(oldOption);
	setCanSendEvents(true);

	// Update sizer
	getSizer().sortItems([&] (const UISizerEntry& a, const UISizerEntry& b)
	{
		const auto& itemA = std::dynamic_pointer_cast<UIListItem>(a.getPointer());
		const auto& itemB = std::dynamic_pointer_cast<UIListItem>(b.getPointer());

		return itemA->getAbsoluteIndex() < itemB->getAbsoluteIndex();
	});
}

void UITreeList::setSingleRoot(bool enabled)
{
	singleRoot = enabled;
}

bool UITreeList::isSingleRoot() const
{
	return singleRoot;
}

void UITreeList::setAllExpanded(bool expanded)
{
	const bool changed = root.setAllExpanded(*this, expanded);
	if (changed) {
		needsRefresh = true;
	}
}

bool UITreeList::canDragListItem(const UIListItem& listItem)
{
	return isDragEnabled() && (!singleRoot || listItem.getAbsoluteIndex() != 0) && canDragItemId(listItem.getId());
}

void UITreeList::makeParentsOfItemExpanded(const String& id)
{
	const auto [found, modified] = root.expandParentsOfId(id);
	if (modified) {
		refresh();
	}
}

bool UITreeList::setSelectedOptionId(const String& id, SelectionMode mode)
{
	makeParentsOfItemExpanded(id);
	return UIList::setSelectedOptionId(id, mode);
}

void UITreeList::enumerateIdsAndLabels(Vector<String>& ids, Vector<String>& names, Vector<Sprite>& icons) const
{
	root.enumerateIdsAndLabels(ids, names, icons);
}

Vector2f UITreeList::getDragPositionAdjustment(Vector2f pos, Vector2f startPos) const
{
	return pos;
}

UITreeListControls::UITreeListControls(String id, UIStyle style)
	: UIWidget(std::move(id), Vector2f(), UISizer(UISizerType::Horizontal, 0))
{
	styles.emplace_back(std::move(style));
	setupUI();
}

float UITreeListControls::updateGuides(const Vector<int>& itemsLeftPerDepth, bool hasChildren, bool expanded)
{
	const auto& style = styles.at(0);
	auto getSprite = [&](size_t depth) -> Sprite
	{
		const bool leaf = depth == itemsLeftPerDepth.size();
		if (leaf) {
			return style.getSprite("leaf");
		} else {
			const bool deepest = depth == itemsLeftPerDepth.size() - 1;
			const auto left = itemsLeftPerDepth[depth];
			if (deepest) {
				if (left == 1) {
					return style.getSprite("guide_l");
				} else {
					return style.getSprite("guide_t");
				}
			} else {
				if (left == 1) {
					return Sprite().setSize(Vector2f(22, 22));
				} else {
					return style.getSprite("guide_i");
				}
			}
		}
	};

	const bool hadChildren = !!expandButton;

	if (waitingConstruction || itemsLeftPerDepth.size() != lastDepth || hasChildren != hadChildren) {
		clear();
		guides.clear();
		lastDepth = itemsLeftPerDepth.size();

		for (size_t i = 1; i < itemsLeftPerDepth.size() + (hasChildren ? 0 : 1); ++i) {
			guides.push_back(std::make_shared<UIImage>(getSprite(i)));
			add(guides.back(), 0, Vector4f(0, -1, 0, 0));
		}

		if (hasChildren) {
			expandButton = std::make_shared<UIButton>("expand", style.getSubStyle("expandButton"));
			collapseButton = std::make_shared<UIButton>("collapse", style.getSubStyle("collapseButton"));

			expandButton->setActive(!expanded);
			collapseButton->setActive(expanded);

			add(expandButton, 0, Vector4f(), UISizerAlignFlags::Centre);
			add(collapseButton, 0, Vector4f(), UISizerAlignFlags::Centre);
		} else if (hadChildren) {
			expandButton->destroy();
			expandButton.reset();
			collapseButton->destroy();
			collapseButton.reset();
		}

		waitingConstruction = false;
		totalIndent = getLayoutMinimumSize(false).x;
	} else {
		// Update guides
		for (size_t i = 1; i < itemsLeftPerDepth.size() + (hasChildren ? 0 : 1); ++i) {
			guides[i - 1]->setSprite(getSprite(i));
		}
	}

	return totalIndent;
}

void UITreeListControls::setExpanded(bool expanded)
{
	if (expandButton) {
		expandButton->setActive(!expanded);
	}
	if (collapseButton) {
		collapseButton->setActive(expanded);
	}
}

void UITreeListControls::setupUI()
{
	setHandle(UIEventType::ButtonClicked, "expand", [=] (const UIEvent& event)
	{
		sendEvent(UIEvent(UIEventType::TreeExpandHandle, getId(), getId()));
	});
	setHandle(UIEventType::ButtonClicked, "collapse", [=](const UIEvent& event)
	{
		sendEvent(UIEvent(UIEventType::TreeCollapseHandle, getId(), getId()));
	});
}

UITreeListItem::UITreeListItem() = default;

UITreeListItem::UITreeListItem(String id, std::shared_ptr<UIListItem> listItem, std::shared_ptr<UITreeListControls> treeCtrl, std::shared_ptr<UILabel> label, std::shared_ptr<UIImage> icon, bool forceLeaf, bool expanded)
	: id(std::move(id))
	, listItem(std::move(listItem))
	, label(std::move(label))
	, icon(std::move(icon))
	, treeControls(std::move(treeCtrl))
	, expanded(expanded)
	, forceLeaf(forceLeaf)
{
	if (treeControls) {
		treeControls->setExpanded(expanded);
	}
}

UITreeListItem* UITreeListItem::tryFindId(const String& id)
{
	if (id == this->id) {
		return this;
	}

	for (auto& c: children) {
		const auto res = c->tryFindId(id);
		if (res) {
			return res;
		}
	}

	return nullptr;
}

UITreeListItem& UITreeListItem::addChild(std::unique_ptr<UITreeListItem> item, size_t pos)
{
	Expects(!forceLeaf);
	
	item->parentId = id;
	item->parent = this;

	const size_t idx = std::min(children.size(), pos);
	children.insert(children.begin() + idx, std::move(item));
	return *children[idx];
}

std::unique_ptr<UITreeListItem> UITreeListItem::removeChild(const String& id)
{
	Expects(!forceLeaf);

	const size_t n = children.size();
	for (size_t i = 0; i < n; ++i) {
		if (children[i]->id == id) {
			auto item = std::move(children[i]);
			children.erase(children.begin() + i);
			item->parentId = "";
			item->parent = nullptr;
			return item;
		}
	}
	throw Exception("No child with id \"" + id + "\"", HalleyExceptions::UI);
}

void UITreeListItem::moveChild(size_t startIndex, size_t targetIndex)
{
	Expects(!forceLeaf);
	
	// If moving forwards, subtract one to account for the fact that the currently occupied slot will be removed
	const size_t finalIndex = std::min(children.size() - 1, targetIndex > startIndex ? targetIndex - 1 : targetIndex);

	// Swap from start to end
	const int dir = finalIndex > startIndex ? 1 : -1;
	for (size_t i = startIndex; i != finalIndex; i += dir) {
		std::swap(children[i], children[i + dir]);
	}
}

void UITreeListItem::setLabel(const LocalisedString& text)
{
	if (label) {
		label->setText(text);
	}
}

void UITreeListItem::setLabelColour(Colour4f colour)
{
	if (label) {
		label->setColour(colour);
	}
}

Colour4f UITreeListItem::getLabelColour() const
{
	if (label) {
		return label->getColour();
	}
	return Colour4f();
}

void UITreeListItem::setIcon(Sprite sprite)
{
	if (icon) {
		icon->setSprite(std::move(sprite));
	}
}

bool UITreeListItem::setExpanded(bool e)
{
	if (!children.empty() && treeControls) {
		const bool changed = e != expanded;
		expanded = e;
		treeControls->setExpanded(e);
		return changed;
	}
	return false;
}

bool UITreeListItem::setAllExpanded(UITreeList& tree, bool expanded)
{
	bool changed = setExpanded(expanded);
	if (changed) {
		tree.sendEvent(UIEvent(UIEventType::TreeItemExpanded, tree.getId(), id, expanded));
	}

	for (auto& c: children) {
		changed = c->setAllExpanded(tree, expanded) || changed;
	}

	return changed;
}

void UITreeListItem::setForceLeaf(bool leaf)
{
	forceLeaf = leaf;
}

std::unique_ptr<UITreeListItem> UITreeListItem::removeFromTree(const String& id)
{
	for (size_t i = 0; i < children.size(); ++i) {
		if (children[i]->id == id) {
			auto res = std::move(children[i]);
			children.erase(children.begin() + i);
			return res;
		}
	}

	for (auto& child: children) {
		auto result = child->removeFromTree(id);
		if (result) {
			return result;
		}
	}

	return {};
}

std::optional<UITreeListItem::FindPositionResult> UITreeListItem::findPosition(UITreeList& tree, Vector2f pos) const
{
	return doFindPosition(tree, pos, 0, true);
}

std::optional<UITreeListItem::FindPositionResult> UITreeListItem::doFindPosition(UITreeList& tree, Vector2f pos, int depth, bool lastBranch) const
{
	if (listItem) {
		const bool isLastItem = lastBranch && (!expanded || children.empty());
		const bool isSingleRootTree = tree.isSingleRoot();
		const bool isRootOfSingleRootTree = depth <= 1 && isSingleRootTree;
	
		const auto r = listItem->getRect();
		const auto b = listItem->getClickableInnerBorder();
		const float x0 = r.getLeft() + b.x;
		const float x1 = r.getRight() - b.z;
		const float y0 = r.getTop() + b.y;
		const float y1 = r.getBottom() - b.w + 1;
		const float h = y1 - y0;
		const float y = pos.y;
		
		if (y >= y0 && y < y1) {
			float threshold0, threshold1;
			if (forceLeaf) {
				threshold0 = y0 + h / 2;
				threshold1 = y0 + h / 2;
			} else if (isRootOfSingleRootTree) {
				threshold0 = y1;
				threshold1 = y0;
			} else {
				threshold0 = y0 + h / 4;
				threshold1 = y0 + 3 * h / 4;
			}
			
			if (y < threshold0 && !isRootOfSingleRootTree) {
				return FindPositionResult(PositionType::Before, this, Rect4f(x0, y0, x1 - x0, 0));
			} else if ((y > threshold1 && !isRootOfSingleRootTree) || forceLeaf) {
				return FindPositionResult(PositionType::After, this, Rect4f(x0, y1, x1 - x0, 0));
			} else {
				assert(!forceLeaf);
				return FindPositionResult(PositionType::OnTop, this, Rect4f(x0, y0, x1 - x0, y1 - y0));
			}
		} else if (y >= y1 && isLastItem && !isSingleRootTree) {
			return FindPositionResult(PositionType::End, nullptr, Rect4f(0, y1, 20, 0));
		}
	}

	if (expanded) {
		for (size_t i = 0; i < children.size(); ++i) {
			auto res = children[i]->doFindPosition(tree, pos, depth + 1, lastBranch && i + 1 == children.size());
			if (res) {
				return *res;
			}
		}
	}
	
	return {};
}

const String& UITreeListItem::getId() const
{
	return id;
}

const String& UITreeListItem::getParentId() const
{
	return parentId;
}

size_t UITreeListItem::getNumberOfChildren() const
{
	return children.size();
}

size_t UITreeListItem::getChildIndex(const String& id) const
{
	for (size_t i = 0; i < children.size(); ++i) {
		if (children[i]->id == id) {
			return i;
		}
	}
	return 0;
}

std::shared_ptr<UIListItem> UITreeListItem::getListItem() const
{
	return listItem;
}

const Vector<std::unique_ptr<UITreeListItem>>& UITreeListItem::getChildren() const
{
	return children;
}

bool UITreeListItem::canHaveChildren() const
{
	return !forceLeaf;
}

std::pair<bool, bool> UITreeListItem::expandParentsOfId(const String& id)
{
	if (id == this->id) {
		return { true, false };
	}

	for (auto& c: children) {
		const auto [containsId, modified] = c->expandParentsOfId(id);
		if (containsId) {
			const bool expanding = !expanded;
			if (expanding) {
				setExpanded(true);
			}
			return { true, modified || expanding };
		}
	}

	return { false, false };
}

std::optional<String> UITreeListItem::getLastExpandedItem(const String& targetId)
{
	return doGetLastExpandedItem(expanded, id, targetId);
}

bool UITreeListItem::addTag(String tag)
{
	if (!hasTag(tag)) {
		tags.push_back(std::move(tag));
		return true;
	}
	return false;
}

bool UITreeListItem::removeTag(const String& tag)
{
	if (hasTag(tag)) {
		std_ex::erase(tags, tag);
		return true;
	}
	return false;
}

bool UITreeListItem::hasTag(const String& tag) const
{
	return std_ex::contains(tags, tag);
}

bool UITreeListItem::hasTagInAncestors(const String& tag) const
{
	return hasTag(tag) || (parent && parent->hasTagInAncestors(tag));
}

std::optional<String> UITreeListItem::doGetLastExpandedItem(bool expandedTree, const String& lastId, const String& targetId)
{
	if (targetId == this->id) {
		return expandedTree ? id : lastId;
	}

	const bool curExpanded = expanded && expandedTree;

	for (auto& c: children) {
		auto value = c->doGetLastExpandedItem(curExpanded, expandedTree ? id : lastId, targetId);
		if (value) {
			return value;
		}
	}

	return {};
}

void UITreeListItem::updateTree(UITreeList& treeList)
{
	Vector<int> itemsLeftPerDepth;
	doUpdateTree(treeList, itemsLeftPerDepth, expanded);
}

void UITreeListItem::collectItems(Vector<std::shared_ptr<UIListItem>>& items)
{
	if (listItem) {
		items.push_back(listItem);
	}

	for (auto& c: children) {
		c->collectItems(items);
	}
}

void UITreeListItem::enumerateIdsAndLabels(Vector<String>& ids, Vector<String>& names, Vector<Sprite>& icons) const
{
	if (!id.isEmpty()) {
		ids.push_back(id);
		names.push_back(label->getTextRenderer().getText());
		icons.push_back(icon->getSprite());
	}

	for (const auto& c: children) {
		c->enumerateIdsAndLabels(ids, names, icons);
	}
}

void UITreeListItem::doUpdateTree(UITreeList& treeList, Vector<int>& itemsLeftPerDepth, bool treeExpanded)
{
	treeList.doSetItemActive(id, treeExpanded);

	if (listItem && treeControls && treeExpanded) {
		const float totalIndent = treeControls->updateGuides(itemsLeftPerDepth, !children.empty(), expanded);
		listItem->setClickableInnerBorder(Vector4f(totalIndent, 0, 0, 0));
	}
	
	itemsLeftPerDepth.push_back(int(children.size()));
	for (auto& c: children) {
		c->doUpdateTree(treeList, itemsLeftPerDepth, expanded && treeExpanded);
		itemsLeftPerDepth.back()--;
	}
	itemsLeftPerDepth.pop_back();
}
