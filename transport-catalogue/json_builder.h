#pragma once

#include "json.h"

namespace json
{

    class KeyContext;
    class StartDictContext;
    class StartArrayContext;

    class Builder
    {
    public:
        const std::vector<Node *> &GetNodesStack() const;
        KeyContext Key(const std::string &s);
        Builder &Value(Node::Value v , int is_ = 1 );
        StartDictContext StartDict();
        StartArrayContext StartArray();
        Builder &EndDict();
        Builder &EndArray();
        Node Build();
        virtual ~Builder(){};

    protected:
        Node root_;
        bool complete = false;
        std::vector<Node *> nodes_stack_ = {nullptr};
        void CheckComplete();
    };

    class KeyContext : public Builder
    {
    public:
        KeyContext(Builder &b) : b_(b) {}
        StartDictContext Value(Node::Value v);
        KeyContext Key(const std::string &s) = delete;
        Node Build() = delete;
        Builder &EndDict() = delete;
        Builder &EndArray() = delete;

    private:
        Builder &b_;
    };

    class StartDictContext : public Builder
    {
    public:
        StartDictContext(Builder &b) : b_(b) {}
        Builder Value(Node::Value v) = delete;
        KeyContext Key(const std::string &s);
        StartDictContext StartDict() = delete;
        StartArrayContext StartArray() = delete;
        Builder &EndArray() = delete;
        Builder &EndDict();
        Node Build() = delete;

    private:
        Builder &b_;
    };

    class StartArrayContext : public Builder
    {
    public:
        StartArrayContext(Builder &b) : b_(b) {}
        StartArrayContext Value(Node::Value v);
        KeyContext Key(const std::string &s) = delete;
        Node Build() = delete;
        Builder &EndDict() = delete;
        Builder &EndArray();

    private:
        Builder &b_;
    };
}