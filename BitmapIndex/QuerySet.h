//
// Created by lukas on 16.03.22.
//

#ifndef BITMAPINDEX_QUERYSET_H
#define BITMAPINDEX_QUERYSET_H

#include <fstream>
#include <cstring>

class QuerySet {
private:
    char ** queries = nullptr;
    unsigned int attrs_count;
public:
    bool is_with_averages = false;
    unsigned int query_count = -1;
    [[nodiscard]] const char * get_query(unsigned int query) const;
    void get_query(unsigned int query, int *, char * key) const;

    static QuerySet* getFromFile(const char *, unsigned int, bool with_averages = false);
    QuerySet(unsigned int count, unsigned int attr_count, bool with_averages = false) {
        query_count = count;
        attrs_count = attr_count;
        queries = new char*[query_count];
        for (int i = 0; i < query_count; ++i) {
            queries[i] = new char [attr_count + with_averages];
        }
    }
};

QuerySet* QuerySet::getFromFile(const char *filename, unsigned int attr_count, bool with_averages) {
    int query_count = 0;
    constexpr int MAX_LEN = 1024;
    char line[MAX_LEN];

    std::ifstream queries(filename);
    queries.getline(line, MAX_LEN);
    sscanf(line, "Query Count:%d", &query_count);
    auto query_set = new QuerySet(query_count, attr_count, with_averages);
    query_set->is_with_averages = with_averages;

    char *cond = nullptr;
    int conditioned_column = 0;
    unsigned int query = 0;
    while (!queries.eof()) {
        conditioned_column = 0;
        cond = nullptr;
        queries.getline(line, MAX_LEN);
        cond = std::strtok(line, ";");
        while (cond != NULL) {
            auto condition_value = std::atoi(cond);
            query_set->queries[query][conditioned_column] = condition_value;
            cond = std::strtok(NULL, ";");
            conditioned_column += 1;
        }
        query += 1;
    }
    queries.close();

    return query_set;
}

const char * QuerySet::get_query(unsigned int query_id) const {
    return queries[query_id];
}

void QuerySet::get_query(unsigned int query_id, int * attr_pos, char *key) const {
    auto keyIdx = 0;
    for (int i = 0; i < attrs_count + is_with_averages; ++i) {
        if (attr_pos[i] > 0) {
            key[keyIdx] = queries[query_id][i];
            keyIdx += 1;
        }
    }
}


#endif //BITMAPINDEX_QUERYSET_H
