package edu.cs300;
import CtCILibrary.*;
import java.util.concurrent.*;
import java.util.ArrayList;
import java.io.*;
import java.util.Scanner;
import java.util.*;

public class PassageProcessor{
    public static void main(String[] args) {

        ArrayList<String> passageNames = new ArrayList<String>();
    
        try{
            BufferedReader read = new BufferedReader(new FileReader("passages.txt"));
            String FileName = read.readLine();
            while(FileName != null){
                passageNames.add(FileName);
                FileName = read.readLine();
            }
            read.close();
        }
        catch (FileNotFoundException fnfe){
            System.out.println("file not found");
        }
        catch (IOException ioe){
            ioe.printStackTrace();
        }
       
        ArrayList<ArrayList<String>> words = new ArrayList<ArrayList<String>>();
        for(int i = 0; i < passageNames.size(); i++){
            try{
                File file = new File(passageNames.get(i));
                Scanner input = new Scanner(file);
                input.useDelimiter("[^a-zA-Z'-]");
                ArrayList<String> row = new ArrayList<>();
                while(input.hasNext()){
                    String word = input.next().toLowerCase();                
                    if(word.length() >= 3 && !word.contains("'") && !word.contains("-")){
                        row.add(word);
                    }
                }
                words.add(row);
            } catch (FileNotFoundException fofe){
                System.out.println("file not found");
            }
        }

        ArrayBlockingQueue[] workers = new ArrayBlockingQueue[passageNames.size()];
        Worker[] workerArr = new Worker[passageNames.size()];

        for(int i = 0; i < passageNames.size(); i++){
            workers[i] = new ArrayBlockingQueue(10);
        }

        for(int i = 0; i < passageNames.size(); i++){
            workerArr[i] = new Worker(words.get(i), i, workers[i], passageNames);
            workerArr[i].start();
        }

        MessageJNI obj = new MessageJNI();
        SearchRequest request = obj.readPrefixRequestMsg();
        String prefix = request.prefix;
        int reqID = request.requestID;
        boolean run = true;
        while(run){
            if(reqID == 0){
                run = false;
                new MessageJNI().writeLongestWordResponseMsg(reqID,prefix,0,"","",passageNames.size(),0);
            }
            try{
                for(int i = 0; i < passageNames.size(); i++){
                    workers[i].put(request);
                }
            } catch (InterruptedException ie){}
            if(run != false){
                request = obj.readPrefixRequestMsg();
                prefix = request.prefix;
                reqID = request.requestID;
            }
        }

        for(int i = 0; i < passageNames.size(); i++){
            try{
                workerArr[i].join();
            } catch(InterruptedException ie){};
        }
   }
}