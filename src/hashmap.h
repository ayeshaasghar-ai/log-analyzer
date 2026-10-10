#pragma once
#include <string>
#include <vector>
#include <utility>

// Apna hash map: key = string, value = int (ginti).
// Collision handling: chaining (har bucket ek chhoti list).
class StringIntMap {
public:
    StringIntMap() : buckets_(16), size_(0) {}

    // key ki ginti 1 badhao (nayi key ho to 1 se shuru)
    void increment(const std::string& key) {
        if (size_ + 1 > buckets_.size() * 3 / 4) {
            rehash(buckets_.size() * 2);
        }
        std::vector<Entry>& bucket = buckets_[indexFor(key, buckets_.size())];
        for (size_t i = 0; i < bucket.size(); i++) {
            if (bucket[i].first == key) {
                bucket[i].second++;
                return;
            }
        }
        bucket.push_back(Entry(key, 1));
        size_++;
    }

    // key ki ginti lo (na mile to 0)
    int get(const std::string& key) const {
        const std::vector<Entry>& bucket = buckets_[indexFor(key, buckets_.size())];
        for (size_t i = 0; i < bucket.size(); i++) {
            if (bucket[i].first == key) {
                return bucket[i].second;
            }
        }
        return 0;
    }

    size_t size() const { return size_; }
    size_t bucketCount() const { return buckets_.size(); }

    // saari (key, ginti) jodiyan ek list mein
    std::vector<std::pair<std::string, int>> items() const {
        std::vector<std::pair<std::string, int>> out;
        for (size_t i = 0; i < buckets_.size(); i++) {
            for (size_t j = 0; j < buckets_[i].size(); j++) {
                out.push_back(buckets_[i][j]);
            }
        }
        return out;
    }

private:
    typedef std::pair<std::string, int> Entry;

    std::vector<std::vector<Entry>> buckets_;
    size_t size_;

    static size_t hashString(const std::string& s) {
        size_t h = 5381;
        for (size_t i = 0; i < s.size(); i++) {
            h = h * 33 + static_cast<unsigned char>(s[i]);
        }
        return h;
    }

    static size_t indexFor(const std::string& key, size_t bucketTotal) {
        return hashString(key) % bucketTotal;
    }

    // dabbe badhao aur saari keys naye dabbon mein dobara rakho
    void rehash(size_t newCount) {
        std::vector<std::vector<Entry>> fresh(newCount);
        for (size_t i = 0; i < buckets_.size(); i++) {
            for (size_t j = 0; j < buckets_[i].size(); j++) {
                const Entry& e = buckets_[i][j];
                fresh[indexFor(e.first, newCount)].push_back(e);
            }
        }
        buckets_.swap(fresh);
    }
};