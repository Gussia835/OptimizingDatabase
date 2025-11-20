#pragma once

#include <iostream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <memory>
#include <set>
#include <queue>
#include "abstract_database.h"

namespace std {
    template <typename T1, typename T2>
    struct hash<std::pair<T1, T2>> {
        size_t operator()(const std::pair<T1, T2>& p) const noexcept {
            return hash<T1>{}(p.first) ^ (hash<T2>{}(p.second) << 1);
        }
    };
}

class OptimizedDatabase : public AbstractDatabase {
private:
    std::unordered_map<int, User> users;
    std::unordered_map<std::pair<int, int>, Post> posts;

    struct CompareByDate {
        bool operator()(const Post& a, const Post& b) const {
            return std::tie(a.Date, a.Id) < std::tie(b.Date, b.Id); 
        }
    };

    struct CompareByLikes {
        bool operator()(const Post& a, const Post& b) const {
            return a.Likes > b.Likes;
        }
    };

    struct CompareByReposts {
        bool operator()(const Post& a, const Post& b) const {
            return a.Reposts > b.Reposts; 
        }
    };

    std::unordered_map<int, std::multiset<Post, CompareByDate>> owner_posts;

public:

    //
    //получение пользователя
    //
    const User& get_user(int id) override {
        auto it = users.find(id);
        if (it != users.end()) return it->second;
        throw DatabaseException("User not found");
    }

    //
    //добавление пользователя
    //
    void insert_user(const User& user) override {
        users[user.Id] = user;
    }

    //
    //Получение поста
    //
    const Post& get_post(int ownerId, int postId) override {
        auto key = std::make_pair(ownerId, postId);
        auto it = posts.find(key);
        if (it != posts.end()) 
            return it->second;
        throw DatabaseException("Post not found");
    }

    //
    //Вставка поста
    //
    void insert_post(const Post& post) override {
        auto key = std::make_pair(post.OwnerId, post.Id);
        if (posts.emplace(key, post).second) {
            owner_posts[post.OwnerId].insert(post);
        }
    }

    //
    //Удаление поста
    //
    void delete_post(int ownerId, int postId) override {
        auto key = std::make_pair(ownerId, postId);
        auto post_it = posts.find(key);
        if (post_it != posts.end()) {
            const Post& post = post_it->second;
            auto& owner_set = owner_posts[ownerId];

            auto it = owner_set.find(post);
            if (it != owner_set.end())
            {
                owner_set.erase(it);
            }
                

            if (owner_set.empty()) {
                owner_posts.erase(ownerId);
            }
            posts.erase(post_it);
        }
        else 
        {
            throw DatabaseException("Post not found");
        }
    }

    //
    //лайк поста
    //
    void like_post(int ownerId, int postId) override {
        auto key = std::make_pair(ownerId, postId);
        auto it = posts.find(key);
        if (it != posts.end()) {
            it->second.Likes++;
        }
    }

    //
    //отмена лайка
    //
    void unlike_post(int ownerId, int postId) override {
        auto key = std::make_pair(ownerId, postId);
        auto it = posts.find(key);
        if (it != posts.end()) {
            it->second.Likes--;
        }
    }
    //
    //Репост
    //
    void repost_post(int ownerId, int postId) override {
        auto key = std::make_pair(ownerId, postId);
        auto it = posts.find(key);
        if (it != posts.end()) {
            it->second.Reposts++;
        }
    }

    //
    //Посты по лайкам
    //
    std::vector<Post> top_k_post_by_likes(int k, int ownerId, int dateBegin, int dateEnd) override {
        auto owner_it = owner_posts.find(ownerId);
        if (owner_it == owner_posts.end()) 
            return {};

        const auto& posts_set = owner_it->second;
        Post dummy_start{};
        dummy_start.Date = dateBegin;
        Post dummy_end{};
        dummy_end.Date = dateEnd;

        auto begin = posts_set.lower_bound(dummy_start);
        auto end = posts_set.upper_bound(dummy_end);

        std::priority_queue<Post, std::vector<Post>, CompareByLikes> min_heap;

        for (auto it = begin; it != end; ++it) {
            if (min_heap.size() < k) {
                min_heap.push(*it);
            }
            else if (it->Likes > min_heap.top().Likes) {
                min_heap.pop();
                min_heap.push(*it);
            }
        }

        std::vector<Post> result;
        while (!min_heap.empty()) {
            result.push_back(min_heap.top());
            min_heap.pop();
        }
        std::reverse(result.begin(), result.end());
        return result;
    }
    //
    //Посты по репостам
    //
    std::vector<Post> top_k_post_by_reposts(int k, int ownerId, int dateBegin, int dateEnd) override 
    {
        auto owner_it = owner_posts.find(ownerId);
        if (owner_it == owner_posts.end()) 
            return {};

        const auto& posts_set = owner_it->second;
        Post dummy_start{};
        dummy_start.Date = dateBegin;
        Post dummy_end{};
        dummy_end.Date = dateEnd;

        
        auto begin = posts_set.lower_bound(dummy_start);
        auto end = posts_set.upper_bound(dummy_end);

        std::priority_queue<Post, std::vector<Post>, CompareByReposts> min_heap;

        for (auto it = begin; it != end; ++it) 
        {
                if (min_heap.size() < k) 
                {
                    min_heap.push(*it);
                }
                else if (it->Reposts > min_heap.top().Reposts) 
                {
                    min_heap.pop();
                    min_heap.push(*it);
                }
            
        }

        std::vector<Post> result;
        while (!min_heap.empty()) 
        {
            result.push_back(min_heap.top());
            min_heap.pop();
        }
        std::reverse(result.begin(), result.end());
        return result;
    }
    //
    //Авторы по лайкам
    //
    std::vector<UserWithLikes> top_k_authors_by_likes(int k, int ownerId, int dateBegin, int dateEnd) override 
    {
        auto owner_it = owner_posts.find(ownerId);
        if (owner_it == owner_posts.end()) 
            return {};

        const auto& posts_set = owner_it->second;
        Post dummy_start{};
        dummy_start.Date = dateBegin;
        Post dummy_end{};
        dummy_end.Date = dateEnd;

        auto begin = posts_set.lower_bound(dummy_start);
        auto end = posts_set.upper_bound(dummy_end);

        std::unordered_map<int, int> likes_count;
        for (auto it = begin; it != end; ++it) 
        {
            likes_count[it->FromId] += it->Likes;
        }

        using Pair = std::pair<int, int>;
        std::priority_queue<Pair, std::vector<Pair>, std::greater<Pair>> min_heap;

        for (const auto& pair : likes_count) 
        {
            int userId = pair.first;
            int likes = pair.second;

            if (min_heap.size() < k) 
            {
                min_heap.emplace(likes, userId);
            }
            else if (likes > min_heap.top().first) 
            {
                min_heap.pop();
                min_heap.emplace(likes, userId);
            }
        }

        std::vector<UserWithLikes> result;
        while (!min_heap.empty()) 
        {
            std::pair<int, int> pair = min_heap.top();
            min_heap.pop();
            result.emplace_back(UserWithLikes{ get_user(pair.second), pair.first });
        }
        std::reverse(result.begin(), result.end());
        return result;
    }
    //
    //Авторы по репостам
    //
    std::vector<UserWithReposts> top_k_authors_by_reports(int k, int ownerId, int dateBegin, int dateEnd) override {
        auto owner_it = owner_posts.find(ownerId);
        if (owner_it == owner_posts.end()) 
            return {};

        const auto& posts_set = owner_it->second;
        Post dummy_start{};
        dummy_start.Date = dateBegin;
        Post dummy_end{};
        dummy_end.Date = dateEnd;

        auto begin = posts_set.lower_bound(dummy_start);
        auto end = posts_set.upper_bound(dummy_end);

        std::unordered_map<int, int> reposts_count;
        for (auto it = begin; it != end; ++it) 
        { 
           reposts_count[it->FromId] += it->Reposts;
           
        }

        using Pair = std::pair<int, int>;
        std::priority_queue<Pair, std::vector<Pair>, std::greater<Pair>> min_heap;

        for (const auto& pair : reposts_count) 
        {
            int reps = pair.second;
            int userId = pair.first;
            if (min_heap.size() < k) 
            {
                min_heap.emplace(reps, userId);
            }
            else if (reps > min_heap.top().first) 
            {
                min_heap.pop();
                min_heap.emplace(reps, userId);
            }
        }

        std::vector<UserWithReposts> result;
        while (!min_heap.empty()) 
        {
            std::pair<int, int> pair = min_heap.top();
            min_heap.pop();
            result.emplace_back(UserWithReposts{ get_user(pair.second), pair.first });
        }
        std::reverse(result.begin(), result.end());
        return result;
    }
};