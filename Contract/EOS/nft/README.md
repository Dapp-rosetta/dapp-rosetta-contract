# 一 dgoods 
[https://github.com/MythicalGames/dgoods/](https://github.com/MythicalGames/dgoods/)

https://github.com/MythicalGames/dgoods/blob/master/dGoods%E5%8D%8F%E8%AE%AE%E4%B8%AD%E6%96%87%E7%89%88.pdf   《dGoods协议中文版.pdf》
提出人是游戏公司Mythical Games(Jan 7, 2019),从暴雪游戏出来的人搞的，（官网https://dgoods.org/）现在有很多钱包方表示支持（tokenpocket,scatter,meet.one等）
```
1 CREATE : create 函数将实例化一个代币。它是在任何代币进行发行前，或者设置
诸如代币类别、名字、最大供应量、谁有权限进行发行代币，是否是非同质化的等
等属性前，必须执行的一个方法。此外，第一次调用该方法时，代币符号会被记
录，并且在后续调用时，必须指定改符号。符号必须是大写 A-Z 组成，请最长为
7 位字符。Name 类型是一个 a-z, 1-5 组成的最长 12 位字符的字符串。


ACTION create(name issuer, string symbol, name category, name
token_name, bool fungible, bool burnable, bool transferable, int64_t
max_supply);
—————————————————————————————————
2 ISSUE : issue 函数可以发行一个代币，并且将所有权指定给账户名’to’. 对于一
个有效的函数调用，代币的符号、类别、名字都必须都先被创建好了。如果是非同
质化或半同质化的代币，则数量必须等于 1，如果是同质化代币，则数量必须是大
于等于 0.0001

ACTION issue(name to, string symbol, name category, name token_name,
double quantity, string metadata_uri, string memo);
—————————————————————————————————
3 PAUSEXFER : 暂停所有代币的所有转账，仅有合约可以调用该函数。如果 pause
参数值为 true，则暂停转账；否则，不暂停。


ACTION pausexfer(bool pause);
—————————————————————————————————
4 BURNNFT : 该函数可用于销毁第三方代币并且释放内存，只有代币的拥有者
(owner) 可以调用该方法，并且 burnable 参数值必须为 true


ACTION burnnft(name owner, vector<uint64_t> tokeninfo_ids);
—————————————————————————————————
5 BURN : 该方法用于销毁同质化代币，并且在所有代币都销毁后释放内存
(RAM)，只有拥有者 (owner) 可以调用该方法，并且 burnable 参数值必须为true

ACTION burn(name owner, uint64_t global_id, double quantity);
—————————————————————————————————
6 TRANSFERNFT : 用于非同质化代币的转账。并且允许通过传递代币的 id 列表进
行批量转账。仅有代币拥有者 (owner) 可以调用改函数并且 transferable 参数值
必须为 true。


ACTION transfernft(name from, name to, vector<uint64_t> tokeninfo_ids,
string memo);
—————————————————————————————————
7 TRANSFER : 该标准的转账方法仅用于同质化代币；区别于将每个代币都一一区
分，这里转账的代币由一个 global_id 进行区分，并且后面跟着希望发送的代币数
量。

ACTION transfer(name from, name to, uint64_t global_id, double quantity);
```

# 二 Simple Assets 
[https://github.com/cryptolions/SimpleAssets](https://github.com/cryptolions/SimpleAssets)

https://medium.com/@cryptolions/eosio%E5%8C%BA%E5%9D%97%E9%93%BE%E6%95%B0%E5%AD%97%E4%BA%A7%E5%93%81-simple-assets-alpha-%E7%AE%80%E4%BB%8B-71e26d4cbb71
《EOSIO区块链数字产品 “Simple Assets”（Alpha）简介》

2019 CryptoLions提出（Feb 24, 2019），已有公司站队，提出者CryptoLions也是eos节点之一。

ACTION
```
 # -- For Non-Fungible Tokens ---
 
 create          (author, category, owner, idata, mdata, requireсlaim)  
 update          (author, owner, assetid, mdata)  
 transfer        (from, to , [assetid1,..,assetidn], memo)  
 burn            (owner, [assetid1,..,assetidn], memo)  
 
 offer           (owner, newowner, [assetid1,..,assetidn])  
 canceloffer     (owner, [assetid1,..,assetidn])  
 claim           (claimer, [assetid1,..,assetidn])  
  
 regauthor       (name author, data, stemplate)  
 authorupdate    (author, data, stemplate)  
 
 delegate        (owner, to, [assetid1,..,assetidn], untildate)  
 undelegate      (owner, from, [assetid1,..,assetidn])  
 
 
 # -- For Fungible Tokens ---
 
 createf         (author, maximum_supply, authorctrl)
 issuef          (to, author, quantity, memo)
 transferf       (from, to, author, quantity, memo)
 burnf           (from, author, quantity, memo)

 openf           (owner, author, symbol, ram_payer)
 closef          (owner, author, symbol)
```

# 三 unicoeos nft
 [https://github.com/unicoeos/eosio.nft](https://github.com/unicoeos/eosio.nft)

简评：unicoeos公司搞的，提出时间比较早( 21 May 2018)，但更新比较慢。看来没有成为标准的希望。

ACTION
```
	/// Creates token with a symbol name for the specified issuer account.
	/// Throws if token with specified symbol already exists.
	/// @param issuer Account name of the token issuer
	/// @param symbol Symbol code of the token
        ACTION create(name issuer, std::string symbol);

	/// Issues specified number of tokens with previously created symbol to the account name "to". 
	/// Each token is generated with an unique token_id assigned to it. Requires authorization from the issuer.
	/// Any number of tokens can be issued.
	/// @param to Account name of tokens receiver
	/// @param quantity Number of tokens to issue for specified symbol (positive integer number)
	/// @param uris Vector of URIs for each issued token (size is equal to tokens number)
	/// @param name Name of issued tokens (max. 32 bytes)
	/// @param memo Action memo (max. 256 bytes)
        ACTION issue(name to,
                   asset quantity,
                   vector<string> uris,
		   string name,
                   string memo);

	/// Transfers 1 token with specified "id" from account "from" to account "to".
	/// Throws if token with specified "id" does not exist, or "from" is not the token owner.
	/// @param from Account name of token owner
	/// @param to Account name of token receiver
	/// @param id Unique ID of the token to transfer
	/// @param memo Action memo (max. 256 bytes)
        ACTION transferid(name from,
                      name to,
                      id_type id,
                      string memo);

	/// Transfers 1 token with specified symbol in asset from account "from" to account "to".
	/// Throws if amount is not 1, token with specified symbol does not exist, or "from" is not the token owner.
	/// @param from Account name of token owner
	/// @param to Account name of token receiver
	/// @param quantity Asset with 1 token 
	/// @param memo Action memo (max. 256 bytes)
	ACTION transfer(name from,
                      name to,
                      asset quantity,
                      string memo);

	/// @notice Burns 1 token with specified "id" owned by account name "owner".
	/// @param owner Account name of token owner
	/// @param id Unique ID of the token to burn
	ACTION burn(name owner,
                  id_type token_id);

	/// @notice Sets owner of the token as a ram payer for stored data.
	/// @param payer Account name of token owner
	/// @param id Unique ID of the token to burn
        ACTION setrampayer(name payer, 
			   id_type id);
```

# 四cocosbcx的NHAS-1808
全球著名手机游戏引擎团队cocos搞的，暂未完全开源，兼容EOS
https://github.com/Cocos-BCX/NHAS-1808

https://dev.cocosbcx.io/docs/bcx-nhas-1808%E6%A0%87%E5%87%86  《标准说明文档》
```
合约对1808 标准资产的操作
非同质资产所有权转移

非同质资产转移 – 调用者

◼ 函数原型
void transfer_nht_from_caller(string to, string token_hash_or_id)

◼ 调用说明
从合约调用者转移非同质资产到账户to

◼ 参数说明
to：目标账户，token_hash_or_id：指定的非同质资产hash 值或者id 编号
非同质资产转移 – 所有者

◼ 函数原型
void transfer_nht_from_owner(string to, string token_hash_or_id)

◼ 调用说明
从合约所有者转移非同质资产到账户to

◼ 参数说明
to：目标账户，token_hash_or_id：指定的非同质资产hash 值或者id 编号
4.1.2. 修改非同质资产数据(特定数据域内)

非同质资产转移 – 所有者

◼ 函数原型
void nht_describe_change(string nht_hash_or_id, string key, string value)

◼ 调用说明
修改非同质资产的合约相关描述，修改部分为合约对应的域

◼ 参数说明
token_hash_or_id：指定的非同质资产hash 值或者id 编号，key：描述项索引，value：索引
对应的描述信息
```