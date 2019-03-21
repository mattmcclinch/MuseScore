//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2008-2019 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#import <Cocoa/Cocoa.h>
#include "touchbar.h"

//---------------------------------------------------------
//   TouchBarItem
//---------------------------------------------------------

@interface TouchBarItem : NSCustomTouchBarItem

@property QAction* action;

- (void)doAction;

@end

@implementation TouchBarItem

- (void)doAction
      {
      if (_action->isCheckable())
            _action->setChecked(!_action->isChecked());
      emit _action->triggered();
      }

@end

//---------------------------------------------------------
//   TouchBarProvider
//---------------------------------------------------------

@interface TouchBarProvider: NSResponder <NSTouchBarDelegate>

@property (strong) NSString* identifier;
@property (strong) NSMutableArray* itemIdentifiers;
@property (strong) NSMutableDictionary* items;

- (instancetype)initWithIdentifier:(NSString*)identifier;
- (NSTouchBar*)makeTouchBar;
- (NSTouchBarItem*)touchBar:(NSTouchBar*)touchBar makeItemForIdentifier:(NSTouchBarItemIdentifier)identifier;

@end

@implementation TouchBarProvider

- (instancetype)initWithIdentifier:(NSString*)identifier
      {
      self = [super init];
      _identifier = identifier;
      _itemIdentifiers = [[NSMutableArray alloc] init];
      _items = [[NSMutableDictionary alloc] init];
      return self;
      }

- (NSTouchBar*)makeTouchBar
      {
      NSTouchBar *bar = [[NSTouchBar alloc] init];
      bar.delegate = self;
      bar.defaultItemIdentifiers = _itemIdentifiers;
      bar.customizationAllowedItemIdentifiers = _itemIdentifiers;
      bar.customizationIdentifier = _identifier;
      return bar;
      }

- (NSTouchBarItem*)touchBar:(NSTouchBar*)touchBar makeItemForIdentifier:(NSTouchBarItemIdentifier)identifier
      {
      return [_items objectForKey:identifier];
      }

@end

//---------------------------------------------------------
//   TouchBar
//---------------------------------------------------------

TouchBar::TouchBar(QString identifierPrefix, QString name)
      {
      _identifierPrefix = identifierPrefix;
      _provider = [[TouchBarProvider alloc] initWithIdentifier:(identifierPrefix + name).toNSString()];
      }

void TouchBar::addButton(QString name, QAction* a, QString description)
      {
      NSString* identifier = (_identifierPrefix + name).toNSString();
      TouchBarItem* item = [[TouchBarItem alloc] initWithIdentifier:identifier];
      NSButton* button = [NSButton buttonWithTitle:a->text().toNSString() target:item action:@selector(doAction)];
      item.view = button;
      item.customizationLabel = description.toNSString();
      item.action = a;
      TouchBarProvider* provider = static_cast<TouchBarProvider*>(_provider);
      [provider.itemIdentifiers addObject:identifier];
      [provider.items setObject:item forKey:identifier];
      }

void TouchBar::setAsTouchBarForWidget(QWidget* widget)
      {
      NSView* view = reinterpret_cast<NSView*>(widget->winId());
      if ([view respondsToSelector:@selector(touchBar)]) {
            TouchBarProvider* provider = static_cast<TouchBarProvider*>(_provider);
            [view setTouchBar:[provider touchBar]];
            }
      }
void TouchBar::setAsTouchBarForApplication()
      {
      NSApplication* application = [NSApplication sharedApplication];
      if ([application respondsToSelector:@selector(touchBar)]) {
            TouchBarProvider* provider = static_cast<TouchBarProvider*>(_provider);
            [application setTouchBar:[provider touchBar]];
            }
      }
