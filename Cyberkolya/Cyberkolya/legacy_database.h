#pragma once
#include <iostream>
#include <vector>
#include <algorithm>
#include "abstract_database.h"

// Говнокод, перед прочтением сжечь
class LegacyDatabase: public AbstractDatabase {
private:
	std::vector<Post> posts;
	std::vector<User> users;

	auto get_post_it(int ownerId, int postId) {
		return std::find_if(posts.begin(), posts.end(),
			[=](const auto& other) {
				return other.Id == postId && other.OwnerId == ownerId;
			}
		);
	}

	auto get_user_it(int id) {
		return std::find_if(users.begin(), users.end(), 
			[=](const auto& other) {
				return other.Id == id;
			}
		);
	}

public:
	void dump_tsv_posts(std::wostream& stream) {
		stream << "id\tdate\towner_id\tfrom_id\tlikes\treposts\n";
		for (const auto& post : posts) stream << post << "\n";
	}

	void dump_tsv_users(std::wostream& stream) {
		stream << "id\tfirst_name\tlast_name\n";
		for (const auto& user : users) stream << user << "\n";
	}

	const Post& get_post(int ownerId, int postId) override {
		auto post = get_post_it(ownerId, postId);
		if (post != posts.end()) return *post;
		throw DatabaseException("Post not found");
	}

	const User& get_user(int id) override {
		auto user = get_user_it(id);
		if (user != users.end()) return *user;
		throw DatabaseException("User not found");
	}

	void insert_post(const Post& post) override {
		// Костыль из-за дубликатов событий при сбоях
		if (get_post_it(post.OwnerId, post.Id) == posts.end()) {
			posts.push_back(post);
		}
	}

	void insert_user(const User& user) override {
		if (get_user_it(user.Id) == users.end()) {
			users.push_back(user);
		}
	}

	void delete_post(int ownerId, int postId) override {
		auto post = get_post_it(ownerId, postId);
		if (post != posts.end()) posts.erase(post);
	}

	void like_post(int ownerId, int postId) override {
		auto post = get_post_it(ownerId, postId);
		if (post != posts.end()) post->Likes++;
	}

	void unlike_post(int ownerId, int postId) override {
		auto post = get_post_it(ownerId, postId);
		if (post != posts.end()) post->Likes--;
	}

	void repost_post(int ownerId, int postId) override {
		auto post = get_post_it(ownerId, postId);
		if (post != posts.end()) post->Reposts++;
	}

	std::vector<Post> top_k_post_by_likes(int k, int ownerId, int dateBegin, int dateEnd) override {
		std::sort(posts.begin(), posts.end(), [=](const auto& p1, const auto& p2) { return p1.Likes > p2.Likes; });
		std::vector<Post> topK;
		for (const auto& post : posts) {
			if (dateBegin <= post.Date && post.Date < dateEnd && post.OwnerId == ownerId)
				topK.push_back(post);
			if (topK.size() == k) break;
		}
		return topK;
	}

	std::vector<Post> top_k_post_by_reposts(int k, int ownerId, int dateBegin, int dateEnd) override {
		std::sort(posts.begin(), posts.end(), [=](const auto& p1, const auto& p2) { return p1.Reposts > p2.Reposts; });
		std::vector<Post> topK;
		for (const auto& post : posts) {
			if (dateBegin <= post.Date && post.Date < dateEnd && post.OwnerId == ownerId)
				topK.push_back(post);
			if (topK.size() == k) break;
		}
		return topK;
	}

	std::vector<UserWithLikes> top_k_authors_by_likes(int k, int ownerId, int dateBegin, int dateEnd) override {
		std::vector<std::pair<int, int>> authorLikes;
		for (const auto& post : posts) {
			if (dateBegin <= post.Date && post.Date < dateEnd && post.OwnerId == ownerId) {
				auto author = std::find_if(authorLikes.begin(), authorLikes.end(),
					[=](const auto& autorToLikes) { return autorToLikes.first == post.FromId; });
				if (author == authorLikes.end()) authorLikes.push_back(std::make_pair(post.FromId, post.Likes));
				else author->second += post.Likes;
			}
		}
		std::sort(authorLikes.begin(), authorLikes.end(), [=](const auto& a1, const auto& a2) { return a1.second > a2.second; });
		if (authorLikes.size() > k)
			authorLikes.resize(k);
		std::vector<UserWithLikes> statistics(authorLikes.size());
		for (auto i = size_t(0); i < authorLikes.size(); i++) {
			statistics[i].User = get_user(authorLikes[i].first);
			statistics[i].Likes = authorLikes[i].second;
		}
		return statistics;
	}

	std::vector<UserWithReposts> top_k_authors_by_reports(int k, int ownerId, int dateBegin, int dateEnd) override {
		std::vector<std::pair<int, int>> authorReposts;
		for (const auto& post : posts) {
			if (dateBegin <= post.Date && post.Date < dateEnd && post.OwnerId == ownerId) {
				auto author = std::find_if(authorReposts.begin(), authorReposts.end(),
					[=](const auto& authorToReposts) { return authorToReposts.first == post.FromId; });
				if (author == authorReposts.end()) authorReposts.push_back(std::make_pair(post.FromId, post.Reposts));
				else author->second += post.Reposts;
			}
		}
		std::sort(authorReposts.begin(), authorReposts.end(), [=](const auto& a1, const auto& a2) { return a1.second > a2.second; });
		if (authorReposts.size() > k)
			authorReposts.resize(k);
		std::vector<UserWithReposts> statistics(authorReposts.size());
		for (auto i = size_t(0); i < authorReposts.size(); i++) {
			statistics[i].User = get_user(authorReposts[i].first);
			statistics[i].Reposts = authorReposts[i].second;
		}
		return statistics;
	}
};
