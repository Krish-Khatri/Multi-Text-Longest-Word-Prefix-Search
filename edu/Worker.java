package edu.cs300;
import CtCILibrary.*;
import java.util.concurrent.*;
import java.util.ArrayList;

class Worker extends Thread{

  Trie textTrieTree;
  ArrayBlockingQueue searchRequestArray;
  int id;
  ArrayList<String> passageNames;

  public Worker(ArrayList<String> words,int id, ArrayBlockingQueue SearchRequestObj , ArrayList<String> passageNames){
    this.textTrieTree=new Trie(words);
    this.searchRequestArray= SearchRequestObj;
    this.id=id;
    this.passageNames = passageNames;
  }

  public void run() {
    System.out.println("Worker-"+this.id+" ("+this.passageNames.get(this.id)+") thread started ...");
    while (true){
      try {
        SearchRequest request = (SearchRequest)this.searchRequestArray.take();
        String prefix = (String)request.prefix;
        if(prefix.equals("   ")){
          break;
        }
        boolean found = this.textTrieTree.contains(prefix);
        String longestWord = this.textTrieTree.longestPrefixWord(prefix);
        if (!found){
          System.out.println("Worker-"+this.id+" "+request.requestID+":"+ prefix+" ==> not found ");
          new MessageJNI().writeLongestWordResponseMsg(request.requestID,prefix,this.id ,passageNames.get(this.id), longestWord, passageNames.size(), 0);
        } else{
          new MessageJNI().writeLongestWordResponseMsg(request.requestID,prefix,this.id,passageNames.get(this.id), longestWord, passageNames.size(), 1);
        }
      } catch(InterruptedException e){
        System.out.println(e.getMessage());
      }
    }
  }
}