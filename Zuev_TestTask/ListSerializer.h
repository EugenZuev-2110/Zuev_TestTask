#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>
#include "ListNode.h"

class ListSerializer {
public:
    void Serialize(ListNode* head, const std::string& filepath) {
        std::ofstream os(filepath, std::ios::binary);
        if (!os) return;

        auto nodes = EnumerateList(head);
        auto nodeMap = BuildAddressMap(nodes);

        WriteSize(os, nodes.size());
        for (auto* node : nodes) {
            WriteNode(os, node, nodeMap);
        }
    }

    ListNode* Deserialize(const std::string& filepath) {
        std::ifstream is(filepath, std::ios::binary);
        if (!is) return nullptr;

        size_t count = ReadSize(is);
        if (count == 0) return nullptr;

        std::vector<ListNode*> nodes = CreateEmptyNodes(count);
        std::vector<int> randIndices;

        for (size_t i = 0; i < count; ++i) {
            randIndices.push_back(ReadNode(is, nodes[i]));
        }

        RestoreLinks(nodes, randIndices);
        return nodes.front();
    }

private:
    std::vector<ListNode*> EnumerateList(ListNode* head) {
        std::vector<ListNode*> nodes;
        for (auto* curr = head; curr; curr = curr->next) {
            nodes.push_back(curr);
        }
        return nodes;
    }

    std::unordered_map<ListNode*, int> BuildAddressMap(const std::vector<ListNode*>& nodes) {
        std::unordered_map<ListNode*, int> map;
        for (int i = 0; i < nodes.size(); ++i) {
            map[nodes[i]] = i;
        }
        return map;
    }

    void WriteNode(std::ostream& os, ListNode* node, const std::unordered_map<ListNode*, int>& map) {
        int randIdx = (node->rand) ? map.at(node->rand) : -1;
        uint32_t dataLen = static_cast<uint32_t>(node->data.size());

        os.write(reinterpret_cast<const char*>(&dataLen), sizeof(dataLen));
        os.write(node->data.data(), dataLen);
        os.write(reinterpret_cast<const char*>(&randIdx), sizeof(randIdx));
    }

    std::vector<ListNode*> CreateEmptyNodes(size_t count) {
        return std::vector<ListNode*>(count, nullptr);
    }

    int ReadNode(std::istream& is, ListNode*& node) {
        node = new ListNode();
        uint32_t dataLen;
        is.read(reinterpret_cast<char*>(&dataLen), sizeof(dataLen));

        node->data.resize(dataLen);
        is.read(&node->data[0], dataLen);

        int randIdx;
        is.read(reinterpret_cast<char*>(&randIdx), sizeof(randIdx));
        return randIdx;
    }

    void RestoreLinks(std::vector<ListNode*>& nodes, const std::vector<int>& randIndices) {
        for (size_t i = 0; i < nodes.size(); ++i) {
            if (i > 0) nodes[i]->prev = nodes[i - 1];
            if (i < nodes.size() - 1) nodes[i]->next = nodes[i + 1];

            int rIdx = randIndices[i];
            if (rIdx != -1) nodes[i]->rand = nodes[rIdx];
        }
    }

    void WriteSize(std::ostream& os, size_t size) {
        uint32_t s = static_cast<uint32_t>(size);
        os.write(reinterpret_cast<const char*>(&s), sizeof(s));
    }

    size_t ReadSize(std::istream& is) {
        uint32_t s = 0;
        is.read(reinterpret_cast<char*>(&s), sizeof(s));
        return static_cast<size_t>(s);
    }

    public:
        ListNode* LoadFromText(const std::string& filepath) {
            std::ifstream is(filepath);
            if (!is) return nullptr;

            auto rawData = ReadLines(is);
            if (rawData.empty()) return nullptr;

            std::vector<ListNode*> nodes = CreateRawNodes(rawData);
            LinkNodes(nodes, rawData);

            return nodes.front();
        }

private:
    struct RawNode {
        std::string data;
        int randIdx;
    };

    std::vector<RawNode> ReadLines(std::ifstream& is) {
        std::vector<RawNode> result;
        std::string line;

        while (std::getline(is, line)) {
            size_t sep = line.find_last_of(';');
            if (sep != std::string::npos) {
                result.push_back({
                    line.substr(0, sep),
                    std::stoi(line.substr(sep + 1))
                    });
            }
        }
        return result;
    }

    std::vector<ListNode*> CreateRawNodes(const std::vector<RawNode>& data) {
        std::vector<ListNode*> nodes;
        nodes.reserve(data.size());
        for (const auto& item : data) {
            ListNode* node = new ListNode();
            node->data = item.data;
            nodes.push_back(node);
        }
        return nodes;
    }

    void LinkNodes(std::vector<ListNode*>& nodes, const std::vector<RawNode>& rawData) {
        for (size_t i = 0; i < nodes.size(); ++i) {
            if (i > 0) nodes[i]->prev = nodes[i - 1];
            if (i < nodes.size() - 1) nodes[i]->next = nodes[i + 1];

            int rIdx = rawData[i].randIdx;
            if (rIdx >= 0 && rIdx < static_cast<int>(nodes.size())) {
                nodes[i]->rand = nodes[rIdx];
            }
        }
    }

public:
    void Clear(ListNode* head) {
        while (head) {
            ListNode* temp = head;
            head = head->next;
            delete temp;
        }
    }
};