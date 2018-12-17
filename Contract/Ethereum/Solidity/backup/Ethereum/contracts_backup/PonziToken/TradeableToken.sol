pragma solidity ^0.4.24;

import "openzeppelin-solidity/contracts/token/ERC20/StandardToken.sol";
/**
 * @title Tradeable Token
 * @dev Standard Token which can be trade within the contract.
 * we support buy() and sell() function in a simpilified bancor algorithm which CW = 50%.
 */
contract TradeableToken is StandardToken {

    uint256 public tokenPriceInitial_ =     0.0000001 ether; // per wei
    uint256 public tokenPriceIncremental_ = 0.00000001 ether; // per wei
    uint256 public depositPool_ = 0 ether;    
    uint256 constant public OFFSET = 2**64; 
    
    // 

    // Event
    event OnBuy(
        address indexed customerAddress,
        uint256 incomingether,
        uint256 tokensMinted
    );
    
    event OnSell(
        address indexed customerAddress,
        uint256 tokensBurned,
        uint256 etherEarned
    );
    
    event OnWithdraw(
        address indexed customerAddress,
        uint256 etherWithdrawn
    );

    // Internal Function
    function _mint(address _customerAddress, uint256 _amount) internal {
        require(_amount > 0 && (SafeMath.add(_amount, totalSupply_) > totalSupply_));
        totalSupply_ = SafeMath.add(totalSupply_, _amount);
        balances[_customerAddress] = SafeMath.add(balances[_customerAddress], _amount);        
    }

    function _burn(address _customerAddress, uint256 _amount) internal {
        require(_amount > 0 && (SafeMath.sub(_amount, totalSupply_) < totalSupply_));
        totalSupply_ = SafeMath.sub(totalSupply_, _amount);
        balances[_customerAddress] = SafeMath.sub(balances[_customerAddress], _amount);        
    }    
    
    function _buy(uint256 _incomingEther) internal returns(uint256) {
        address _customerAddress = msg.sender;
        depositPool_ = SafeMath.add(depositPool_, _incomingEther);        
        uint256 _amountOfTokens = etherToTokens_(_incomingEther);
        _mint(_customerAddress, _amountOfTokens);
        emit OnBuy(_customerAddress, _incomingEther, _amountOfTokens);        
        return _amountOfTokens;        
    }

    function _sell(uint256 _incomingToken) internal returns(uint256) {
        address _customerAddress = msg.sender;
        require(_incomingToken <= balances[_customerAddress]);
        uint256 _amountOfEther = tokensToEther_(_incomingToken);
        _burn(_customerAddress, _incomingToken);
        depositPool_ = SafeMath.sub(depositPool_, _amountOfEther);                
        _customerAddress.transfer(_amountOfEther);
        emit OnSell(msg.sender, _incomingToken, _amountOfEther);
        return _amountOfEther;
    }

    // Read Only
    /**
     * Return the buy price of 1 individual token.
     */
    function sellPrice() 
        public 
        view 
        returns(uint256)
    {
        // our calculation relies on the token supply, so we need supply. Doh.
        if(totalSupply_ == 0){
            return tokenPriceInitial_ - tokenPriceIncremental_;
        } else {
            uint256 _ether = tokensToEther_(1e18);
            return _ether;
        }
    }
    
    /**
     * Return the sell price of 1 individual token.
     */
    function buyPrice() 
        public 
        view 
        returns(uint256)
    {
        // our calculation relies on the token supply, so we need supply. Doh.
        if(totalSupply_ == 0){
            return tokenPriceInitial_ + tokenPriceIncremental_;
        } else {
            uint256 _ether = tokensToEther_(1e18);
            return _ether;
        }
    }    
    /**
    * @dev Gets the token price
    * @return uint256 representing the token price
    */
    function getPrice() public view returns (uint256) {
        if(totalSupply_ == 0){
            return tokenPriceInitial_;
        } else {
            return SafeMath.div(SafeMath.mul(depositPool_, 2e18), totalSupply_);
        }
    }

    // Public Function
    /**
    * @dev Buy some token
    */
    function buy() public payable {
        _buy(msg.value);
    }

    /**
    * @dev Sell some token
    */
    function sell(uint256 _amount) public {
        _sell(_amount);
    }	

    /**
     * @dev Fallback function to handle ether that was send straight to the contract
     */
    function() public payable {
        _buy(msg.value);
    }    

    /**
    * @dev calculate how many token will be minted when sending amount of ether.
    * We use a simplified bancor algorithm which CW = 50% to deal with this.
    * https://storage.googleapis.com/website-bancor/2018/04/01ba8253-bancor_protocol_whitepaper_en.pdf
    * @return uint256 representing the amount of token will be minted.
    */
    function etherToTokens_(uint256 _ether)
        public
        view
        returns(uint256 _tokens)
    {
        uint256 _tokenPriceInitial = tokenPriceInitial_ * 1e18;
        uint256 _tokensReceived = 
         (
            (
                // underflow attempts BTFO
                SafeMath.sub(
                    (sqrt
                        (
                            (_tokenPriceInitial**2)
                            +
                            (2*(tokenPriceIncremental_ * 1e18)*(_ether * 1e18))
                            +
                            (((tokenPriceIncremental_)**2)*(totalSupply_**2))
                            +
                            (2*(tokenPriceIncremental_)*_tokenPriceInitial*totalSupply_)
                        )
                    ), _tokenPriceInitial
                )
            )/(tokenPriceIncremental_)
        )-(totalSupply_)
        ;
       // require(_tokensReceived == etherToTokens2_(_ether));
        return _tokensReceived;
    }

    /**
     * @dev This should be a better implementation.
     */
    function etherToTokens2_(uint256 _ether)
        public
        view
        returns(uint256)
    {
        uint256 _tokenPrice = getPrice() * 1e18;
        uint256 _tokensReceived = 
         (
            (
                // underflow attempts BTFO
                SafeMath.sub(
                    (sqrt
                        (
                            (_tokenPrice**2)
                            +
                            (2*(tokenPriceIncremental_ * 1e18)*(_ether * 1e18))
                        )
                    ), _tokenPrice
                )
            )/(tokenPriceIncremental_)
        )
        ;
  
        return _tokensReceived;
    }    

    /**
    * @dev be aware, due to precision issue this two function didn't have consistency with each other.
    * @return uint256 representing the ether which will be release.
    */
    function tokensToEther_(uint256 _tokens)
        internal
        view
        returns(uint256 _ether)
    {

        uint256 tokens_ = (_tokens + 1e18);
        uint256 totalSupply_ = (totalSupply() + 1e18);
        uint256 _etherReceived =
        (
            // underflow attempts BTFO
            SafeMath.sub(
                (
                    (
                        (
                            tokenPriceInitial_ +(tokenPriceIncremental_ * (totalSupply_/1e18))
                        )-tokenPriceIncremental_
                    )*(tokens_ - 1e18)
                ),(tokenPriceIncremental_*((tokens_**2-tokens_)/1e18))/2
            )
        /1e18);
        return _etherReceived;
    }
       
    /**
    * @dev calculate sqrt() via Newton's method.
    * https://en.wikipedia.org/wiki/Fast_inverse_square_root#Newton's_method
    */
    function sqrt(uint x) internal pure returns (uint y) {
        uint z = (x + 1) / 2;
        y = x;
        while (z < y) {
            y = z;
            z = (x / z + z) / 2;
        }
    }    
}