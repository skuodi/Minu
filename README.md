# Minu

Minimal, header-only library for text-based graphical menus.
`Minimal + menu = Minu`

## Features

- Header-only library
- Minimal (500 LOC including lots of helpful comments)
- No explicit dynamic memory allocation
- 3-tier structure ( Menu -> Page -> Page Item)
- Callback functions for page and item transitions
- Hardware-agnostic
  - the only hardware-dependent code is are user-defined print and inverted print functions
- Items have an auxiliary text section whose colour can be user-defined
- Info pages that allow user-defined content to be rendered on screen

## Concepts

There are 3 fundamental constructs that make up a Minu:
- `MinuPageItem`
  * The fundamental building block of the menu system that contains the text to be displayed and occupies one line on the display.
  * Each `MinuPageItem` comprises a main and an auxiliary text section.
  * The main text accounts for the first (often larger) portion of the item's line
  * The actual length of these two portions is configurable when initializing the parent `Minu` that a `MinuPageItem` will be a child of.
  * The auxiliary text section can be printed in user-defined colour.
  * A callback function may be defined for when the item becomes the highlighted item of the parent page.
  * There is also provision for a link funciton associated with the item, which may be used to perform a task when the item is selected

- `MinuPage`
  * Contains a collection of one or more `MinuPageItem`s.
  * Callback funcitons may be defined and will be automatically called when the page:
    1. becomes the currently active page of the `Minu` (page opened callback)
    2. is removed as the currently active page of the `Minu` (page closed callback)
    3. has completed being rendered as the currently active page of the `Minu`

- `Minu`
  * This is the parent menu struture that contains child pages which, in turn, contain child items.

## Usage

1. Create an instance of a `Minu` object.
```c++
  Minu menu(printText, printTextInverted, MINU_MAIN_TEXT_LEN, MINU_AUX_TEXT_LEN);

```

2. Create a page and add items to it.
```c++

  MinuPage homePage("HOMEPAGE", menu.numPages());
  homePage.addItem(goToWiFiPage, "Wi-Fi", " ", updateWiFiItem);
  homePage.addItem(goToPowerPage, "Power", NULL);

```

3. Set callback functions for the page
```c++
  homePage.setOpenedCallback(pageOpenedCallback);
  homePage.setClosedCallback(pageClosedCallback);
  homePage.setRenderedCallback(pageRenderedCallback);
```

4. Add the page to the menu
```c++
   menu.addPage(homePage);
```

**Note:**

For menu navigation, the following functions are available:
- `Minu`
  - `goToPage()` selects the page that should be set as the currently active page.
  - `render()` performs the actual rendering of the menu

- `MinuPage.`
  - `highlightNextItem()` selects the next registered child item to be the currently active item of the page
  - `highlightPreviousItem()` selects the previous registered child item to be the currently active item of the page

## Demo
![Demo](examples/Minu_Example_M5Stick-CPlus2.gif)