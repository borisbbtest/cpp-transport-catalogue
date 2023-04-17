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
        return {*this};
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

    ////////////////    BuildContext    //////////////////


    BuildContext BuildContext::Value(Node::Value v)
    {
        CheckComplete();
        Node tmp;
        const_cast<Node::Value &>(tmp.GetValue()) = std::move(v);
        const_cast<Array &>(nodes_stack_.back()->AsArray()).emplace_back(std::move(tmp));
        if (nodes_stack_.size() == 0)
        {
            complete = true;
        }
        return {*this};
    }

    ValueAfterKeyContext KeyContext::Value(Node::Value v)
    {
       // return GetB().ValueAfterKey(v);
        auto* tmp = &GetB() ;
        CheckComplete();
        const_cast<Node::Value &>((*tmp->nodes_stack_.back()).GetValue()) = std::move(v);
        tmp->nodes_stack_.pop_back();
        if (tmp->nodes_stack_.size() == 0)
        {
            tmp->complete = true;
        }
        return {*tmp};
    }

    KeyContext BuildContext::Key(const std::string &s)
    {
        return b_.Key(s);
    }

    Node BuildContext::Build()
    {
        return b_.Build();
    }
    Builder &BuildContext::GetB()
    {
        return b_;
    }

    //////////////////    Other contexts    ///////////////////////

    /// \\ \\ 

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

}