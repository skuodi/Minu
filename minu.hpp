/*
 * @file  minu.hpp
 * @brief Header-only implementation of a minimal text-based menu system.
 *
 */

#ifndef _LIBMINU_H_
#define _LIBMINU_H_

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <vector>

#define MINU_ITEM_TEXT_SEPARATOR_DEFAULT  "|"   // Separates the main and auxiliary text sections of a menu item
#define MINU_TITLE_PADDING_DEFAULT        "_"   // Padding character for page title
#define MINU_FOREGROUND_COLOUR_DEFAULT    WHITE	// Text foreground default colour
#define MINU_BACKGROUND_COLOUR_DEFAULT    BLACK	// Text background default colour
#define MINU_AUX_TEXT_LEN_DEFAULT         0
#define MINU_MAIN_TEXT_LEN_DEFAULT        10

/// @brief Generic callback function executed when a menu event occurs
typedef void (*MinuCallbackFunction)(void *);						

/// @brief      Basic function for printing text to the display
/// @param msg  Text to be printed
/// @param len  Number of characters to print.
/// @note       If \a len is less than the length of \a msg, the text should be truncated to len
/// @note       If \a len is greater than the length of \a msg, the text should padded with spaces to len characters
/// @param fore Text foreground colour
/// @param back Text background colour
typedef void (*MinuPrintFunction)(const char * msg, uint8_t len, uint16_t fore, uint16_t back);

// ssize_t is undefined on Arduino
#ifdef ARDUINO
typedef int ssize_t;
#endif 

/// @brief Most basic unit of the menu system.
class MinuPageItem
{

public:
  /// @brief Class constructor for creating a valid empty item;
  MinuPageItem()
  {
    // Since the link and highlightedCallback are functions, they must be initialized to NULL by default
    // to prevent function calling at random memory addressess
    this->_link = NULL;
    this->_highlightedCallback = NULL;
    this->_auxTextForeground = MINU_FOREGROUND_COLOUR_DEFAULT;
    this->_auxTextBackground = MINU_BACKGROUND_COLOUR_DEFAULT;
  }

  /// @brief Class constructor
  /// @param id       Unique identifier assigned to the item on creation
  /// @param link     User-defined function associated with the item.
  /// @note           The \a link is not called automatically at runtime, and must explicitly be invoked by the user
  /// @param mainText The text that occupies the greater portion of the item's line when printed.
  /// @param auxText  The text that occupies the smaller portion of the item's line when printed.
  /// @note           The colour of the \a auxText can be customized to indicate a special status
  /// @param hCb      Function to be called automatically when the item becomes the highlighted member of the parent page
  /// @param auxFore  Custom foreground colour used for printing the \a auxText
  /// @param auxBack  Custom background colour used for printing the \a auxText
  MinuPageItem(ssize_t id, MinuCallbackFunction link, const char *mainText, const char *auxText,
               MinuCallbackFunction hCb = NULL, uint16_t auxFore = MINU_FOREGROUND_COLOUR_DEFAULT,
               uint16_t auxBack = MINU_BACKGROUND_COLOUR_DEFAULT)
  {
    this->_id = id;
    this->_link = link;
    this->_highlightedCallback = hCb;
    this->_auxTextForeground = auxFore;
    this->_auxTextBackground = auxBack;

    this->setAuxText(auxText);
    this->setMainText(mainText);
  }
    
  /// @brief    Set the function to be called when the item becomes the highlighted item of the currently active page.
  /// @param cb User-defined callback function.
  /// @note     When called, the pointer to this item is passed as the parameter.
  void setHighlightedCallback(MinuCallbackFunction cb){ this->_highlightedCallback = cb;}

  /// @brief Invoke the highlighted callback (if one was registered)
  void callHighlightedCallback(void)
  {
    if (this->_highlightedCallback)
      this->_highlightedCallback(this);
  }

  /// @brief Returns the user-defined function associated with the item
  ///        which can be called to perform a task when the item is selected
  MinuCallbackFunction link(void) const { return this->_link; };

  ///@brief Returns the identifier of assigned to the item  unique within a MinuPage
  size_t id(void) const { return _id; };

  /// @brief Set the main text of the item
  /// @param mainText Text to set.
  /// @note  Setting the \a mainText to NULL clears the current text.
  void setMainText(const char *mainText) {this->_mainText = (mainText) ? mainText : String();}

  /// @brief Set the auxiliary text of the item
  /// @param auxText Text to set.
  /// @note  Setting the \a auxText to NULL clears the current text.
  void setAuxText(const char *auxText){this->_auxText = (auxText) ? auxText : String();}

  /// @brief Returns the item's main text
  const char* mainText(void)const {return this->_mainText.c_str();}

  /// @brief  Copies the item's main text into a user-provided buffer
  /// @param  buff User-provided buffer
  /// @param  buff_size Size of the user-provied buffer
  /// @return true, if the main text was successfully copied into the buffer
  /// @return false, if the parameters were invalid or if the main text is empty
  bool getMainText(char *buff, size_t buff_size)
  {
    if (!buff || !buff_size || !this->_mainText.length())
      return false;

    strncpy(buff, this->_mainText.c_str(), buff_size);
    return true;
  }

  /// @brief Returns the length of the item's main text
  size_t mainTextLength() { return this->_mainText.length();}

  /// @brief Return the item's auxiliary text
  const char* auxText(void)const {return this->_auxText.c_str();}

  /// @brief  Copies the item's auxiliary text into a user-provided buffer
  /// @param  buff User-provided buffer
  /// @param  buff_size Size of the user-provied buffer
  /// @return true, if the auxiliary text was successfully copied into the buffer
  /// @return false, if the parameters were invalid or if the auxiliary text is empty
  bool getAuxText(char *buff, size_t buff_size)
  {
    if (!buff || !buff_size || !this->_auxText.length())
      return false;

    strncpy(buff, this->_auxText.c_str(), buff_size);
    return true;
  }

  /// @brief Returns the length of the item's auxiliary text
  size_t auxTextLength() { return this->_auxText.length(); }

  /// @brief Set the foreground colour used to print the auxiliary text
  void setAuxTextForeground(uint16_t fore) { this->_auxTextForeground = fore; }

  /// @brief Set the background colour used to print the auxiliary text
  void setAuxTextBackground(uint16_t back) { this->_auxTextBackground = back; }

  /// @brief Return the foreground colour used to print the auxiliary text
  uint16_t auxTextForeground() const { return this->_auxTextForeground; }

  /// @brief Return the background colour used to print the auxiliary text
  uint16_t auxTextBackground() const { return this->_auxTextBackground; }

private:
  String _mainText;
  String _auxText;
  uint16_t _auxTextForeground;
  uint16_t _auxTextBackground;
  MinuCallbackFunction _link;
  size_t _id;
  MinuCallbackFunction _highlightedCallback;
};

class MinuPage
{

public:
  /// @brief Class constructor.
  /// @param title    Text to be printed at the top of the page
  /// @param id       User-assigned unique identifier for the page within the Minu
  /// @param infoMode Whether to skip the printing of child items when rendering the page
  /// @note           When the info mode flag is set, only the title is printed when the page is rendered
  ///                 regardless of the number of items the page has.
  ///                 This allows the user to print custom information to the screen
  MinuPage(const char *title, size_t id, bool infoMode = false)
  {
    this->_id = id;
    this->_highlightedIndex = 0;
    this->_infoMode = infoMode;
    
    this->_openedCallback = NULL;
    this->_renderedCallback = NULL;
    this->_closedCallback = NULL;
    setTitle(title);
  }
  /// @brief  Set the function to be called when the page becomes the currently active page
  /// @param  cb User-defined callback function.
  /// @note   When called, the pointer to this page is passed as the parameter.
  void setOpenedCallback(MinuCallbackFunction cb) { this->_openedCallback = cb; }
  
  /// @brief  Set the function to be called when this page is fully rendered
  /// @param  cb User-defined callback function.
  /// @note   When called, the pointer to this page is passed as the parameter.
  void setRenderedCallback(MinuCallbackFunction cb) { this->_renderedCallback = cb; }
  
  /// @brief  Called for the currently active page when changing to a different page
  /// @param  cb User-defined callback function.
  /// @note   When called, the pointer to this page is passed as the parameter.
  void setClosedCallback(MinuCallbackFunction cb) { this->_closedCallback = cb; }

  /// @brief Set the text to be printed at the top of the page
  void setTitle(const char *title) {this->_title = title;}

  /// @brief Invoke the page opened callback (if one was registered)
  void callOpenedCallback()
  {
    if (this->_openedCallback)
      this->_openedCallback(this);
  }

  /// @brief Invoke the page rendered callback (if one was registered)
  void callRenderedCallback()
  {
    if (this->_renderedCallback)
      this->_renderedCallback(this);
  }

  /// @brief Invoke the page closed callback (if one was registered)
  void callClosedCallback()
  {
    if (this->_closedCallback)
      this->_closedCallback(this);
  }
  
  ///@brief Uniquely identifies the page within a Minu 
  size_t id(void) const { return _id; };

  ///@brief Highlight the item with the next index within a page.
  ///@note  If the current item is the last in the list, the index wraps around to the first item.
  ///@note  If the page has no items, this function has no effect.
  ssize_t highlightNextItem(void)
  {
    if(!this->_items.size())
      return -1;
    // Increment the current index
    this->_highlightedIndex++;

    // If the index overflows or becomes greater than the number of items in the page, reset the index
    if (this->_highlightedIndex < 0 || this->_highlightedIndex >= this->_items.size())
      this->_highlightedIndex = 0;
    
    // Call the highlighted callback for the new highlighted item
    this->_items[this->_highlightedIndex].callHighlightedCallback();
    
    // Return the index of the new highlighted item
    return this->_highlightedIndex;
  }

  ///@brief Returns the index of the currently highlighted item within the page
  ssize_t highlightedIndex(void) const { return this->_highlightedIndex; };

  ///@brief Highlights the item with the given index within the page
  bool highlightItem(size_t index)
  {
    // Confirm that the index is valid
    if (!this->_items.size() || index >= this->_items.size())
      return false;

    this->_highlightedIndex = index;
    
    // Call the highlighted callback for the new highlighted item
    this->_items[this->_highlightedIndex].callHighlightedCallback();
    return true;
  }

  ///@brief Highlight the item with the next index within a page.
  ///@note  If the current item is the first in the list, the index wraps around to the last item.
  ///@note  If the page has no items, this function has no effect.
  ssize_t highlightPreviousItem(void)
  {
    if(!this->_items.size())
      return -1;
    // decrement the current index
    this->_highlightedIndex--;

    // If the index underflows, reset the index to the last item of the page
    if (this->_highlightedIndex < 0)
      this->_highlightedIndex = (this->_items.size()) ? (this->_items.size() - 1) :  0;

    // Call the highlighted callback for the new highlighted item
    this->_items[this->_highlightedIndex].callHighlightedCallback();
    return this->_highlightedIndex;
  }

  /// @brief          Register a new child item  
  /// @param link     User-defined function associated with the item.
  /// @note           The \a link is not called automatically at runtime, and must explicitly be invoked by the user
  /// @param mainText The text that occupies the greater portion of the item's line when printed.
  /// @param auxText  The text that occupies the smaller portion of the item's line when printed.
  /// @note           The colour of the \a auxText can be customized to indicate a special status
  /// @param hCb      Function to be called automatically when the item becomes the highlighted member of the parent page
  ssize_t addItem(MinuCallbackFunction link, const char *mainText, const char *auxText, MinuCallbackFunction hCb = NULL)
  {
    size_t newId = this->_items.size();
    MinuPageItem itm(newId, link, mainText, auxText, hCb);
    this->_items.push_back(itm);

    if (this->_items.size() == newId + 1)
      return newId;

    return -1;
  }

  /// @brief  Delete a registered child item with the provided index
  /// @param  index Index of the item to be deleted
  /// @return true, if the item was successfully deleted
  /// @return false, if the page has no items or the index was invalid
  bool removeItem(size_t index)
  {
    if (!this->_items.size() || index >= this->_items.size())
      return false;

    this->_items.erase(this->_items.begin() + index);
    return true;
  }

  /// @brief Delete all of the page's registered child items
  void removeAllItems(void) {this->_items.clear();}

  /// @brief Returns the item with the given index
  /// @param index Index of the item 
  /// @return Item at given index, if the index is valid
  /// @return Empty item, if the index is invalid
  MinuPageItem getItem(size_t index)
  {
    if (index >= this->_items.size())
      return MinuPageItem();

    return this->_items[index];
  }

  /// @brief Return the number of the page's registered items
  size_t getItemCount() { return this->_items.size(); }

  /// @brief Return a reference the page's currently highlighted child item
  const MinuPageItem &highlightedItem(void) const
  {
    if (_highlightedIndex >= 0 && _highlightedIndex < this->_items.size())
      return this->_items[_highlightedIndex];

    return MinuPageItem();
  }
  /// @brief Returns a reference to the vector of the page's child items
  std::vector<MinuPageItem> &items() { return this->_items; }
  
  /// @brief Returns the page's title
  const char *title() const { return this->_title.c_str(); }
  
  /// @brief Returns the page's infoMode flag
  /// @return 
  bool infoMode() const { return this->_infoMode; }

private:
  bool _infoMode;
  size_t _id;
  String _title;
  std::vector<MinuPageItem> _items;
  ssize_t _highlightedIndex;
  MinuCallbackFunction _openedCallback;
  MinuCallbackFunction _renderedCallback;
  MinuCallbackFunction _closedCallback;
};

class Minu
{

public:
  /// @brief Class condtructor
  /// @param print_txt          Basic used to print text to the screen
  /// @param print_txt_inverted Basic used to print inverted colour text to the screen, used for highlighted items
  /// @param mainTextLen        Length of the main text section of a MinuPageItem
  /// @param auxTextLen         Length of the auxiliary text section of a MinuPageItem
  Minu(MinuPrintFunction print_txt, MinuPrintFunction print_txt_inverted, uint8_t mainTextLen, uint8_t auxTextLen)
  {
    this->_print_txt = print_txt;
    this->_print_txt_inverted = print_txt_inverted;
    this->_mainTextLen = (mainTextLen) ? mainTextLen : MINU_MAIN_TEXT_LEN_DEFAULT;

    this->_auxTextLen = (auxTextLen) ? auxTextLen : MINU_AUX_TEXT_LEN_DEFAULT;
    this->_currentPage = 0;
    this->_rendered = true;
  }
  /// @brief Register a new page to the Minu
  /// @param title Title of the new page
  /// @return Id assigned to the page
  ssize_t addPage(const char *title)
  {
    size_t id = this->_pages.size();
    this->_pages.push_back(new MinuPage(title, id));
    return id;
  }

  /// @brief Register a new page by copy
  ssize_t addPage(MinuPage page)
  {
    size_t id = this->_pages.size();

    this->_pages.push_back(new MinuPage(page));
    return id;
  }

  /// @brief Delete a registered page 
  /// @param index Index of page to be deleted
  /// @return true, if the page is successfully deleted
  /// @return false, if the provided index is invalid
  bool removePage(size_t index)
  {
    if (!this->_pages.size() || index >= this->_pages.size())
      return false;
    auto deletedPage = this->_pages.begin() + index;
    delete(*deletedPage);
    this->_pages.erase(deletedPage);
    return true;
  }
  /// @brief Set the page with the given id to be the currently active page
  /// @return true, on success
  /// @return false, if \a id was invalid
  bool goToPage(size_t id)
  {
    if (!this->_pages.size() || id >= this->_pages.size())
      return false;

    if(this->_currentPage >= 0 && this->_currentPage < this->_pages.size())
      this->_pages[this->_currentPage]->callClosedCallback();

    this->_currentPage = id;
    this->_pages[this->_currentPage]->callOpenedCallback();
    this->_rendered = false;
    return true;
  }
  /// @brief Returns a pointer to the current page
  /// @return Valid pointer, on success
  /// @return NULL, on fail
  MinuPage *currentPage() const
  {
    if (!this->_pages.size() || this->_currentPage < 0 || this->_currentPage >= this->_pages.size())
      return NULL;
    return this->_pages[this->_currentPage];
  }
  /// @brief Returns the index of the currently selected page
  /// @return -1, if the menu has no registered pages
  ssize_t currentPageId() const { return (this->_pages.size()) ? this->_currentPage : -1; }

  /// @brief Return the number of the menu's child pages 
  size_t numPages() const { return this->_pages.size(); }

  /// @brief Return the vector of the menu's child items
  const std::vector<MinuPage *> pages() const { return this->_pages; }
  
  /// @brief Whether or not the menu has been rendered after the selected page changed
  bool rendered()const {return this->_rendered;}

  /// @brief Create a text-based graphical representation of the current page
  /// @param count Maximum number of items to print, one item per line
  void render(uint8_t count)
  {
    // Check that the menu has registered child pages, and that the argument is valid
    if (!this->_pages.size() || this->_currentPage >= this->_pages.size() || !count)
      return;

    ssize_t highlightedIndex = this->_pages[this->_currentPage]->highlightedIndex();
    size_t itemCount = this->_pages[this->_currentPage]->getItemCount();

    char printBuff[256];
    memset(printBuff, 0, sizeof(printBuff));

    // The title text is padded with underscores(_) on either side so we distribute the padding equally on both sides.
    // If the number of underscores is odd, the extra one is post-fixed.
    const uint8_t pageWidth = this->_mainTextLen + this->_auxTextLen;
    uint8_t titleLen = (this->_pages[this->_currentPage]->title()) ? strlen(this->_pages[this->_currentPage]->title()) : 0;
    uint8_t padding = (pageWidth > titleLen) ? (pageWidth - titleLen) : 1;
    uint8_t paddingLeft = padding / 2;
    uint8_t paddingRight = (padding != 1) ? (padding / 2) + (padding % 2) : 0;
    
    if((titleLen + paddingLeft + paddingRight) > sizeof(printBuff))
      titleLen = sizeof(printBuff) - (paddingLeft + paddingRight);
    if (this->_pages[this->_currentPage]->title() && titleLen)
    {
      // Prefix padding
      while (paddingLeft--)
        sprintf(printBuff + strlen(printBuff), MINU_TITLE_PADDING_DEFAULT);

      // Concatenate title
      if(titleLen)
        strncpy(printBuff + strlen(printBuff), this->_pages[this->_currentPage]->title(), titleLen);
      
      // Postfix padding
      while (paddingRight--)
        sprintf(printBuff + strlen(printBuff), MINU_TITLE_PADDING_DEFAULT);

      // Append newline characters
      strcpy(printBuff + strlen(printBuff), "\n\n");

      // Print to screen
      this->_print_txt(printBuff, strlen(printBuff), MINU_FOREGROUND_COLOUR_DEFAULT, MINU_BACKGROUND_COLOUR_DEFAULT);
    }

    // Here we print the page's items
    // The highlighted item is always the first to be rendered so that changing the highlighted index creates a scrolling effect
    for (auto it = highlightedIndex; it < itemCount; ++it)
    {
      // Don't print items if the page has the infoMode flag set
      if (this->_pages[this->_currentPage]->infoMode())
        break;
      
      // If the current index is the index of page's highlighted item, print it inverted
      if (it == highlightedIndex)
      {
        // If the item has no main text, skip it
        if (!this->_pages[this->_currentPage]->getItem(it).getMainText(printBuff, sizeof(printBuff)))
          continue;

        this->_print_txt_inverted(printBuff, (this->_pages[this->_currentPage]->getItem(it).auxTextLength()) ? (this->_mainTextLen) : pageWidth, MINU_FOREGROUND_COLOUR_DEFAULT, MINU_BACKGROUND_COLOUR_DEFAULT);

        // If the item has auxiliary text, print it in custom colour.
        // For proper presentation, an item's auxiliary text is not highlighted. Only the main text is highlighted
        if (this->_pages[this->_currentPage]->getItem(it).getAuxText(printBuff, sizeof(printBuff)))
        {
          this->_print_txt(MINU_ITEM_TEXT_SEPARATOR_DEFAULT, strlen(MINU_ITEM_TEXT_SEPARATOR_DEFAULT), MINU_FOREGROUND_COLOUR_DEFAULT, MINU_BACKGROUND_COLOUR_DEFAULT);
          this->_print_txt(printBuff, this->_pages[this->_currentPage]->getItem(it).auxTextLength(),
                    this->_pages[this->_currentPage]->getItem(it).auxTextForeground(),
                    this->_pages[this->_currentPage]->getItem(it).auxTextBackground());
        }

        this->_print_txt_inverted("\n", strlen("\n"), MINU_FOREGROUND_COLOUR_DEFAULT, MINU_BACKGROUND_COLOUR_DEFAULT);
      }
      else
      {
        // If the item has no main text, skip it
        if (!this->_pages[this->_currentPage]->getItem(it).getMainText(printBuff, sizeof(printBuff)))
          continue;

        this->_print_txt(printBuff, (this->_pages[this->_currentPage]->getItem(it).auxTextLength()) ? (this->_mainTextLen) : pageWidth, MINU_FOREGROUND_COLOUR_DEFAULT, MINU_BACKGROUND_COLOUR_DEFAULT);

        // If the item has auxiliary text, print it in custom colour.
        if (this->_pages[this->_currentPage]->getItem(it).getAuxText(printBuff, sizeof(printBuff)))
        {
          this->_print_txt(MINU_ITEM_TEXT_SEPARATOR_DEFAULT, strlen(MINU_ITEM_TEXT_SEPARATOR_DEFAULT), MINU_FOREGROUND_COLOUR_DEFAULT, MINU_BACKGROUND_COLOUR_DEFAULT);
          this->_print_txt(printBuff, this->_pages[this->_currentPage]->getItem(it).auxTextLength(),
                    this->_pages[this->_currentPage]->getItem(it).auxTextForeground(),
                    this->_pages[this->_currentPage]->getItem(it).auxTextBackground());
        }
        this->_print_txt("\n", strlen("\n"), MINU_FOREGROUND_COLOUR_DEFAULT, MINU_BACKGROUND_COLOUR_DEFAULT);
      }

      // Keep track of how many items have been printed and stop if we've printed the specified maximum
      if (!--count)
        break;
    }
    // The menu has now been rendered
    this->_rendered = true;
    // Call the page's rendered callback functtion
    this->_pages[this->_currentPage]->callRenderedCallback();
  }

private:
  bool _rendered;
  std::vector<MinuPage *> _pages;
  ssize_t _currentPage;
  MinuPrintFunction _print_txt;
  MinuPrintFunction _print_txt_inverted;
  uint8_t _mainTextLen;
  uint8_t _auxTextLen;
};

#endif