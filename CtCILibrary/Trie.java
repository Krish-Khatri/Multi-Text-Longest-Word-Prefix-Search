package CtCILibrary;

import java.util.ArrayList;
import java.util.Stack;


/* Implements a trie. We store the input list of words in tries so
 * that we can efficiently find words with a given prefix. 
 */ 
public class Trie
{
    // The root of this trie.
    private TrieNode root;

    /* Takes a list of strings as an argument, and constructs a trie that stores these strings. */
    public Trie(ArrayList<String> list) {
        root = new TrieNode();
        for (String word : list) {
            root.addWord(word);
        }
    }  
    

    /* Takes a list of strings as an argument, and constructs a trie that stores these strings. */    
    public Trie(String[] list) {
        root = new TrieNode();
        for (String word : list) {
            root.addWord(word);
        }
    }    

    /* Checks whether this trie contains a string with the prefix passed
     * in as argument.
     */
    public boolean contains(String prefix, boolean exact) {
        TrieNode lastNode = root;
        int i = 0;
        for (i = 0; i < prefix.length(); i++) {
            lastNode = lastNode.getChild(prefix.charAt(i));
            if (lastNode == null) {
                return false;	 
            }
        }
        return !exact || lastNode.terminates();
    }
    
    public boolean contains(String prefix) {
    	return contains(prefix, false);
    }
    
     public String longestPrefixWord(String prefix) {
       if(contains(prefix)){
           //Get to last node of prefix
           TrieNode node = root;
           for(int i = 0; i < prefix.length(); i++){
               char data = prefix.charAt(i);
               node = node.getChild(data);
           }

           StringBuffer prefixBuffer = new StringBuffer(prefix);
           String longestWord = new String(findLongestWord(prefixBuffer, node));
           return longestWord;
       }
       return "----";
    }

    public StringBuffer findLongestWord(StringBuffer prefix, TrieNode start){

        StringBuffer tempWord = new StringBuffer("");
        int lengthWord = 0;

        // Prefix is Longest Word
        if(start.terminates() == true){
            tempWord = prefix;
            lengthWord = prefix.length();
        }
        //Iterrate through trie
        for(TrieNode iter : start.getHashMap().values()){
            StringBuffer tempPrefix = new StringBuffer(prefix);
            StringBuffer restWord = findLongestWord(tempPrefix.append(iter.getChar()), iter);
            // If longer word found set tempWord to it
            if(restWord.length() > lengthWord) {
                tempWord = restWord;
                lengthWord = restWord.length();
            // if lengths are equal, find last alphabetically
            } else if(restWord.length() == lengthWord){
                if(restWord.toString().compareTo(tempWord.toString()) > 0) {
                    tempWord = restWord;
                    lengthWord = restWord.length();
                }
            }
        }
        return tempWord;
    }

    public TrieNode getRoot() {
    	return root;
    }
}
