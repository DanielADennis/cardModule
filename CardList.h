#ifndef CARDLIST_H
#define CARDLIST_H

#include "Card.h"
#include "Arduino.h"

struct CardList 
{
  int activeCardIndex;
  int length;
  Card cards[12];
  CardList()
  {
    
  };
  void append(Card card) 
  {
      if (length >= 12)
        remove(0); //remove oldest card
        
      cards[length++] = card;
      activeCardIndex = length-1;  
  }
  
  void next() 
  {
      if (length > 0 && activeCardIndex >= length - 1) 
        activeCardIndex = 0;
      else
        activeCardIndex++;
  }
  
  void prev() 
  {
      if (length > 0 && activeCardIndex == 0)
         activeCardIndex = length-1;  
      else
        activeCardIndex--;
  }
  
  void pop_back() 
  {
      if (activeCardIndex = length-1 && length > 1)
      {
        activeCardIndex--;
      }
        
      if (length > 0)
      {
        for (int i = 0; i < length - 1; i++) 
        {
          cards[i] = cards[i+1];
        }
        //cards[length-1] = "";
        length--;
      }
  }

  void remove(int index) 
  {
      if ((activeCardIndex > index) || ((activeCardIndex == index) && activeCardIndex != 0))
        activeCardIndex--;
      else if ((activeCardIndex == index) && length > 1)
      {
        activeCardIndex++;
      }
      
      if (index >= length) return;
      for (int i = index; i < length - 1; i++) 
      {
        cards[i] = cards[i+1];
      }
      //cards[length-1] = "";
      length--;
  }
  
  Card getCard(int i)
  { 
    if (length > i)
    {
      return cards[i];
    }
    return cards[0];
  }
  
  Card getActiveCard()
  {
    return getCard(activeCardIndex);
  }
};
#endif
