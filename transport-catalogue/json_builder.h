#pragma once

#include "json.h"

namespace json
{

    class BuildContext;
    class KeyContext;
    class StartDictContext;
    class StartArrayContext;
    class ValueContext;
    class ValueAfterKeyContext;
    class ValueAfterStartArrayContext;

    class Builder
    {
    public:
        const std::vector<Node *> &GetNodesStack() const;
        KeyContext Key(const std::string &s);
        ValueContext Value(Node::Value v);
        StartDictContext StartDict();
        StartArrayContext StartArray();
        Builder &EndDict();
        Builder &EndArray();
        Node Build();
        virtual ~Builder(){};
        std::vector<Node *> nodes_stack_ = {nullptr};
        bool complete = false;
    protected:
        Node root_;
        void CheckComplete();
    };

    class BuildContext : public Builder
    {
    public:
        BuildContext(Builder &b) : b_(b){};
        KeyContext Key(const std::string &s);
        BuildContext Value(Node::Value v);
        Node Build();
        Builder &GetB();

    protected:
        Builder &b_;
    };

    class KeyContext : public BuildContext
    {
    public:
        KeyContext(Builder &b) : BuildContext(b) {}
        ValueAfterKeyContext Value(Node::Value v);
        KeyContext Key(const std::string &s) = delete;
        Node Build() = delete;
    };

    class StartDictContext : public BuildContext
    {
    public:
        StartDictContext(Builder &b) : BuildContext(b) {}
        BuildContext Value(Node::Value v) = delete;
        StartDictContext StartDict() = delete;
        StartArrayContext StartArray() = delete;
        Node Build() = delete;
    };

    class StartArrayContext : public BuildContext
    {
    public:
        StartArrayContext(Builder &b) : BuildContext(b) {}
        ValueAfterStartArrayContext Value(Node::Value v);
        KeyContext Key(const std::string &s) = delete;
        Node Build() = delete;
    };

    class ValueContext : public BuildContext
    {
    public:
        ValueContext(Builder &b) : BuildContext(b) {}
        KeyContext Key(const std::string &s) = delete;
        BuildContext Value(Node::Value v) = delete;
    };

    class ValueAfterKeyContext : public BuildContext
    {
    public:
        ValueAfterKeyContext(Builder &b) : BuildContext(b) {}
        BuildContext Value(Node::Value v) = delete;
        Builder EndArray() = delete;
        Node Build() = delete;
    };

    class ValueAfterStartArrayContext : public BuildContext
    {
    public:
        ValueAfterStartArrayContext(Builder &b) : BuildContext(b) {}
        BuildContext Value(Node::Value v);
        KeyContext Key(const std::string &s) = delete;
        Node Build() = delete;
    };

}