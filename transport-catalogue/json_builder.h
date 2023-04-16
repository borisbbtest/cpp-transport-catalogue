#pragma once

#include "json.h"

namespace json
{

    class BuildContext;
    class KeyContext;
    class ValueBuildContext;
    class ValueAfterKeyContext;
    class ValueAfterStartArrayContext;

    class Builder
    {
    public:
        const std::vector<Node *> &GetNodesStack() const;
        KeyContext Key(const std::string &s);
        ValueBuildContext Value(Node::Value v);
        ValueAfterKeyContext ValueAfterKey(Node::Value v);
        ValueAfterStartArrayContext ValueAfterStartArray(Node::Value v);
        Builder StartDict();
        Builder StartArray();
        Builder EndDict();
        Builder EndArray();
        Node Build();
        virtual ~Builder() {}

    private:
        Node root_;
        std::vector<Node *> nodes_stack_ = {nullptr};
        bool complete = false;
        void CheckComplete();
    };

    class BuildContext : public Builder
    {
    public:
        BuildContext(Builder &b);
        KeyContext Key(const std::string &s);
        BuildContext Value(Node::Value v);
        Node Build();
        Builder &GetB();

    private:
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

    class ValueBuildContext : public BuildContext
    {
    public:
        ValueBuildContext(Builder &b) : BuildContext(b) {}
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
        ValueAfterStartArrayContext Value(Node::Value v);
        KeyContext Key(const std::string &s) = delete;
        Node Build() = delete;
    };

}