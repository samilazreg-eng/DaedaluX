#ifndef ELEMENT_STACK_H
#define ELEMENT_STACK_H

#include <list>
#include <memory>
#include <stack>
#include <stdexcept>
#include <set>
#include <stdexcept>

#include <daedalux/core/semantic/variable/state.hpp>

class elementStack {
public:
  struct element {

    element(void);

    explicit element(std::shared_ptr<state> s, unsigned int depth = 0);

    ~element() = default;

    std::shared_ptr<state> current_state;
    std::list<std::shared_ptr<state>> Post;
    bool init;
    unsigned int depth;

    bool hasSuccessors(void) const { return !Post.empty(); }

    /// @brief Returns the next successor of the current state and removes it from the list of successors
    /// @return The next successor of the current state
    std::shared_ptr<state> nextSuccessor(void)
    {
      if (Post.empty())
        throw std::runtime_error("No more successors");
      auto res = Post.front();
      Post.pop_front();
      return res;
    }
  };

  elementStack() = default; // constructors

  ~elementStack() = default; // destructors

  void push(std::shared_ptr<state> s, int depth = 0);

  void pop(void);

  void pop(unsigned int numberOfElements);

  std::shared_ptr<element> top(void) const;

  bool isIn(const element & elem) const;

  bool isIn(unsigned long elem) const;

  bool empty(void) const;

  std::stack<std::shared_ptr<element>> stackElem;
  std::set<unsigned long> setElem;
};

#endif
