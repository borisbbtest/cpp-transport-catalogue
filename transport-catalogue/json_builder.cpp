#include "json_builder.h"

namespace json
{

    void Builder::CheckComplete()
    {
        if (complete == true)
        {
            throw std::logic_error("JSON is already complete");
        }
    }

    const std::vector<Node *> &Builder::GetNodesStack() const
    {
        return nodes_stack_;
    }

    // --------
    StartDictContext Builder::StartDict()
    {
        CheckComplete();
        if (nodes_stack_.back() == nullptr)
        {
            root_ = Node{Dict{}};
            nodes_stack_.back() = &root_;
        }
        else if (nodes_stack_.back()->IsDict())
        {
            throw std::logic_error("Bad map start");
        }
        else if (nodes_stack_.back()->IsArray())
        {
            const_cast<Array &>(nodes_stack_.back()->AsArray()).emplace_back(Node{Dict{}});
            nodes_stack_.emplace_back(const_cast<Node *>(&(nodes_stack_.back()->AsArray().back())));
        }
        else
        {
            *nodes_stack_.back() = Node{Dict{}};
        }
        return StartDictContext(*this);
    }

    StartArrayContext Builder::StartArray()
    {
        CheckComplete();
        if (nodes_stack_.back() == nullptr)
        {
            root_ = Node{Array{}};
            nodes_stack_.back() = &root_;
        }
        else if (nodes_stack_.back()->IsDict())
        {
            throw std::logic_error("Bad array start");
        }
        else if (nodes_stack_.back()->IsArray())
        {
            const_cast<Array &>(nodes_stack_.back()->AsArray()).emplace_back(Node{Array{}});
            nodes_stack_.emplace_back(const_cast<Node *>(&(nodes_stack_.back()->AsArray().back())));
        }
        else
        {
            *nodes_stack_.back() = Node{Array{}};
        }
        return {*this};
    }

    Node Builder::Build()
    {
        if (complete == false)
        {
            throw std::logic_error("JSON is not complete");
        }
        return root_;
    }

    KeyContext Builder::Key(const std::string &s)
    {
        CheckComplete();
        if (nodes_stack_.back() == nullptr || !nodes_stack_.back()->IsDict())
        {
            throw std::logic_error("Key must be within a map and go first");
        }
        nodes_stack_.emplace_back(&(const_cast<Dict &>(nodes_stack_.back()->AsDict())[std::move(s)]));
        return {*this};
    }

    KeyContext StartDictContext::Key(const std::string &s)
    {
        return b_.Key(s);
    }

    StartDictContext KeyContext::Value(Node::Value v)
    {
        return b_.Value(v, 3);
    }

    StartArrayContext StartArrayContext::Value(Node::Value v)
    {
        return b_.Value(v, 2);
    }

    Builder &Builder::Value(Node::Value v, int is_)
    {
        CheckComplete();
        if (is_ == 1)
        {
            const_cast<Node::Value &>(root_.GetValue()) = std::move(v);
            nodes_stack_.pop_back();
            if (nodes_stack_.size() == 0)
            {
                complete = true;
            }
        }
        else if (is_ == 2)
        {
            CheckComplete();
            Node tmp;
            const_cast<Node::Value &>(tmp.GetValue()) = std::move(v);
            const_cast<Array &>(nodes_stack_.back()->AsArray()).emplace_back(std::move(tmp));
            if (nodes_stack_.size() == 0)
            {
                complete = true;
            }
        }
        else if (is_ == 3)
        {
            CheckComplete();
            const_cast<Node::Value &>((*nodes_stack_.back()).GetValue()) = std::move(v);
            nodes_stack_.pop_back();
            if (nodes_stack_.size() == 0)
            {
                complete = true;
            }
            return {*this};
        }
        return {*this};
    }

    Builder &Builder::EndDict()
    {
        CheckComplete();
        if (!nodes_stack_.back()->IsDict())
        {
            throw std::logic_error("Bad map end");
        }
        nodes_stack_.pop_back();
        if (nodes_stack_.size() == 0)
        {
            complete = true;
        }
        return {*this};
    }

    Builder &StartDictContext::EndDict()
    {
        return b_.EndDict();
    }

    Builder &Builder::EndArray()
    {
        CheckComplete();
        if (!nodes_stack_.back()->IsArray())
        {
            throw std::logic_error("Bad array end");
        }
        nodes_stack_.pop_back();
        if (nodes_stack_.size() == 0)
        {
            complete = true;
        }
        return {*this};
    }

    Builder &StartArrayContext::EndArray()
    {
        return b_.EndArray();
    }

}