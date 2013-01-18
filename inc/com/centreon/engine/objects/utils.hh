/*
** Copyright 2011-2013 Merethis
**
** This file is part of Centreon Engine.
**
** Centreon Engine is free software: you can redistribute it and/or
** modify it under the terms of the GNU General Public License version 2
** as published by the Free Software Foundation.
**
** Centreon Engine is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Centreon Engine. If not, see
** <http://www.gnu.org/licenses/>.
*/

#ifndef CCE_OBJECTS_UTILS_HH
#  define CCE_OBJECTS_UTILS_HH

#  include <map>
#  include <string>
#  include <vector>
#  include "com/centreon/engine/namespace.hh"
#  include "com/centreon/engine/objects.hh"

CCE_BEGIN()

namespace       objects {
  namespace     utils {
    template <class T>
    void        remove_object_list(T const* obj, T** head, T** tail) {
      T* current = *head;
      T* prev = NULL;
      while (current != NULL) {
        if (current == obj) {
          if (prev == NULL)
            *head = current->next;
          else
            prev->next = current->next;
          if (current->next == NULL)
            *tail = prev;
          break;
        }
        prev = current;
        current = current->next;
      }
    }

    template<class T>
    inline std::vector<T*> tab2vec(T** tab) {
      std::vector<T*> vec;
      for (unsigned int i = 0; tab[i] != NULL; ++i)
        vec.push_back(tab[i]);
      return (vec);
    }

    inline std::vector<std::string> tab2vec(char** tab) {
      std::vector<std::string> vec;
      for (unsigned int i = 0; tab[i] != NULL; ++i)
        vec.push_back(tab[i]);
      return (vec);
    }
  }
}

CCE_END()

#endif // !CCE_OBJECTS_UTILS_HH
