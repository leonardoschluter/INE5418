package gartic_in_java;

import org.jgroups.*;

import java.io.File;
import java.io.IOException;
import java.net.URL;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.*;
import java.util.stream.Collectors;
import java.util.stream.Stream;

import static gartic_in_java.Chat.*;

public class Jartic implements Receiver {
    JChannel channel;
    Address leader = null;
    String wordToDraw = "";
    Score score = new Score();
    Queue<Answer> roundHistory = new LinkedList<Answer>();
    private Board draw;
    private Chat chat = new Chat(this);
    private JChannel drawChannel;
    private WordDictionary dictionary;
    private Random R = new Random();

    /*
        Flow methods
    */
    private void start() throws Exception {
        createChatChannel();

        score.addPlayer(channel.getAddress());

        Thread thread = new Thread() {
            public void run() {
                try {
                    dispatchDrawApp();
                } catch (Exception e) {
                    System.out.println("Error: did not start draw app");
                    e.printStackTrace();
                }
            }
        };
        thread.start();

        chat.eventLoop();

        thread.join();
        channel.close();
        drawChannel.close();
    }

    private void createChatChannel() throws Exception {

        File path = new File("udp.xml");
        channel = new JChannel(path.getAbsolutePath());
        channel.setReceiver(chat);
        channel.connect("ChatCluster");
    }

    private void dispatchDrawApp() throws Exception {
        File path = new File("udp.xml");
        this.drawChannel = new JChannel(path.getAbsolutePath());
        this.draw = new Board(drawChannel, isLeader());
        this.draw.setClusterName("DrawCluster");
        this.draw.go();
    }
    public void updatePlayersInGame(){
        channel.getView().getMembers().forEach(address -> {
            score.addPlayer(address);
        });
    }
    public void startRound() throws Exception {
        updatePlayersInGame();
        score.updateLeader(channel.getAddress());
        chat.send(MSG_warningStartRound);

        wordToDraw = sortNextWord();
        System.out.println(MSG_wordToDraw + wordToDraw);
        Thread.sleep(1000);
        draw.clearPanel();

        Date ts = new Date();
        chat.send(MSG_roundStarted + ts.getTime());
        Thread.sleep(1000 * 20);
        
        ts = new Date();
        chat.send(MSG_roundFinished + ts.getTime());
        
        finishRound();
    }

    private void finishRound() {
        // Calculate points
        int leaderPoints = calculateAndUpdatePlayerPoints();
        leaderPoints = leaderPoints * 2;

        score.updateScore(leader.toString(), leaderPoints);

        Address address = score.electNewLeader();

        // Share with others the score and new Leader
        chat.send(MSG_updateScore+score.toSerializable());
        chat.send(MSG_newLeader + address.toString());

        // Clear draw, wordToDraw and the answers submited in this round
        resetRound();
    }

    private void resetRound(){
        draw.clearPanel();
        wordToDraw = "";
        roundHistory = new LinkedList<Answer>();
    }

    /*
    State methods
     */

    private int calculateAndUpdatePlayerPoints() {
        int leaderPoints = 0;
        List<Answer> pointedAnswers = new ArrayList<>();
        int pointCounter = 10;
        while (!this.roundHistory.isEmpty()) {
            Answer answer = this.roundHistory.remove();
            answer.setPoints(pointCounter);
            pointedAnswers.add(answer);
            System.out.println("Answer: " + answer.toString());
            if (pointCounter > 1) {
                pointCounter--;
            }
            leaderPoints++;
            score.updateScore(answer.address, answer.getPoints());
        }
        return leaderPoints;
    }

    private Address findAddressByString(String str) {
        List<Address> addresses = channel.getView().getMembers().stream().filter(address -> {
            return address.toString().equals(str);
        }).collect(Collectors.toList());
        return addresses.get(0);
    }

    private void saveAnswer(String[] answer) {
        roundHistory.add(new Answer(answer[0], answer[1]));
        Address address = findAddressByString(answer[0]);
        if (address != null) {
            Message msg = new ObjectMessage(address, MSG_gotItRight);
            try {
                channel.send(msg);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    public boolean isLeader() {
        if (leader == null) {
            // Did set view coordinator as leader
            leader = channel.getView().getCoord();
            System.out.println("No leader, setting view coordinator as leader ->"+leader);
            score.updateLeader(leader);
        }
        Address currentAddress = channel.getAddress();
        return leader.equals(currentAddress);
    }



    public void onUpdateScoreByText(String scoreText) {
        Thread thread = new Thread() {
            public void run() {
                try {
                    score.updateScoreByString(scoreText);
                    return;
                } catch (Exception e) {
                    System.out.println("Error: did not update score");
                    e.printStackTrace();
                }
            }
        };
        thread.start();
    }

    public void onQuit() {
        draw.stop();
        // the channels are close on the end of start, after eventLoop is broken;
    }

    public void onReset() {
        score.resetScore();
        System.out.println(score.toString());
        resetRound();
        if(isLeader()){
            Address address = score.electNewLeader();
            chat.send(MSG_newLeader + address.toString());
        }
    }

    public void onNewLeader(String newLeader) {
        this.leader = findAddressByString(newLeader);
        score.updateLeader(leader);
        if (isLeader()) {
            draw.setCanDraw(true);
            System.out.println("You are the next to draw ! type /start to begin the round");
        } else {
            draw.setCanDraw(false);
        }
    }

    public void onAnswerSubmitted(String line) {
        String[] answer = line.split(":");
        System.out.println("Submitted Answer:" + answer[1] + " - Addr: " + answer[0]);
        // TODO add ts to time of when the answer is received, check the ts to roundStarted to eliminate late answers.
        if (wordToDraw.equals(answer[1])) {
            saveAnswer(answer);
            return;
        }
    }

    public boolean canStartRound() {
        return wordToDraw.isEmpty() && isLeader();
    }

    public WordDictionary loadDictionary(){
        WordDictionary dict = new WordDictionary();
        File path = new File("1000words.txt");
        try (Stream<String> stream = Files.lines(Paths.get(path.getAbsolutePath()))) {
            WordDictionary finalDict = dict;
            stream.forEach(word -> {
                finalDict.words.add(new Word(word));
            });
            dict = finalDict;
        } catch (IOException e) {
            e.printStackTrace();
            System.out.println("Problem loading the dictionary, setting Hardcoded dict");
            dict = new WordDictionary();
            dict.words.add(new Word("hardcoded"));
        }
        return dict;
    }

    /*
    util methods - could be extracted to another file/class
    to keep history of the already sorted words.
     */
    private String sortNextWord() {
        if(dictionary == null){
            dictionary = loadDictionary();
        }
        int sortedIndex = R.nextInt(dictionary.words.size());
        return dictionary.words.get(sortedIndex).word;
    }

    public static void main(String[] args) throws Exception {
        new Jartic().start();
    }
}