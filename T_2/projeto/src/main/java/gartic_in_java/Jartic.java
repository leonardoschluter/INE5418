package gartic_in_java;

import org.jgroups.*;

import java.util.*;
import java.util.stream.Collectors;

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
        channel = new JChannel("/home/leonardo/Documentos/devTools/jgroups/udp.xml");
        channel.setReceiver(chat);
        channel.connect("ChatCluster");
    }

    private void dispatchDrawApp() throws Exception {
        this.drawChannel = new JChannel("/home/leonardo/Documentos/devTools/jgroups/udp.xml");
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
        int leaderPoints = calculateAndUpdatePlayerPoints();
        leaderPoints = leaderPoints * 2;
        score.updateScore(leader.toString(), leaderPoints);
        chat.send(MSG_updateScore+score.toSerializable());

        resetRound();
        Address address = score.electNewLeader();
        chat.send(MSG_newLeader + address.toString());
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
            leader = channel.getView().getCoord();
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

    public void onNewLeader(String newLeader) {
        this.leader = findAddressByString(newLeader);
        if (isLeader()) {
            draw.setCanDraw(true);
            System.out.println("You are the next to draw ! type /start to draw");
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

    /*
    util methods - could be extracted to another file/class
    to keep history of the already sorted words.
     */
    private String sortNextWord() {
        // TODO implement a decent word sorter
/*
        ObjectMapper mapper = new ObjectMapper();

        //JSON file to Java object
        Staff obj = mapper.readValue(new File("c:\\test\\staff.json"), Word.class);

        //JSON URL to Java object
        Staff obj = mapper.readValue(new URL("http://some-domains/api/staff.json"), Staff.class);

        //JSON string to Java Object
        Staff obj = mapper.readValue("{'name' : 'mkyong'}", Staff.class);

 */
        return "hardcoded";
    }

    public static void main(String[] args) throws Exception {
        new Jartic().start();
    }
}